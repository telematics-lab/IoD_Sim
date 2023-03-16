//#include <iostream>
#include <math.h>
//#include <cstring>
//#include <random>
//#include <complex>
//#include <fstream>
//#include <algorithm>
// g++ -c -fPIC ../droneenv_final.cpp -o droneenv.o && g++ -shared -Wl,-soname,libdroneenv.so -o libdroneenv.so  droneenv.o
class IrsAssistedChannnelGain
{
public:
  static void
  CalcGain (double *action, double *ret)
  {
    //ret: [x_d,y_d,z_d,...,x_u,y_u,z_u,...,reward,done,data_u,...,patch_u,..., ]
    const double C = 299792458.0; //Velocità della luce

    const static int Nusers = 7; //Utenti
    const static int Ndrones = 3; //IRS
    const static int P = 4; //Patch per IRS
    int rr[P] = {1, 25, 1, 25}; //
    int RR[P] = {24, 48, 24, 48}; // Suddivisione in patch
    int cc[P] = {1, 1, 25, 25}; //
    int CC[P] = {24, 24, 48, 48}; //
    double d_r = 0.01; //Larghezza PRU (X)
    double d_c = 0.01; //Altezza PRU (Y)
    double s = d_c * d_r; //Area PRU

    double B = 10000; //Larghezza di banda usata per calcolare il rumore
    double sigma_d = pow (10, ((-174 + 10 * log10 (B) - 30) / 10)); // ro^2 ovvero il rumore in mW

    double pos[Ndrones][3] = {
        {0, -150, 50}, {0, -150, 50}, {0, -150, 50}}; //Posizione dei "Ndrones" droni

    double pos_BS[3] = {0, 0, 0}; //Posizione della singola Base Station

    double pos_UE[Nusers][3] = {
        {120, -10, 0},  {50, -100, 0}, {0, 100, 0}, {-100, 20, 0},
        {-120, -50, 0}, {-90, 100, 0}, {70, 80, 0}}; //Posizione dei "Nusers" utenti

    double d_BR[Ndrones], d_RG[Ndrones][Nusers], d_BG[Nusers], etav[Ndrones][Nusers],
        sin_theta_br[Ndrones], cos_phi_br[Ndrones], sin_phi_br[Ndrones],
        sin_theta_rg[Ndrones][Nusers], cos_phi_rg[Ndrones][Nusers], sin_phi_rg[Ndrones][Nusers], invpowcdff[Nusers], rate[Nusers],
        K_BR[Ndrones], K_RG[Ndrones][Nusers];

    //Costanti
    double epsilon = 0.01;
    double invqfunc = 2.3263478740408408; //for eps = 0.01

    double Kmin_db = 6;
    double Kmax_db = 10;
    double Kmin = pow (10, (Kmin_db / 10)); //A1
    double Kmax = pow (10, (Kmax_db / 10));
    double A2 = log (pow (Kmax / Kmin, 2)) / M_PI; //Paper: Formula 13 (Forse c'è un errore)
    double K_BG = Kmin; //K-factor BS-GU

    double f_c = 10.5e9; // Freq. utilizzata (10.5 GHz)

    double beta_BRG = pow (C / f_c, 2) * s / (64 * pow (M_PI, 3)); //Usato per calc. eta in Form. 19
    double beta_BG = pow (C / f_c, 2) / (pow (4 * M_PI, 2)); //Usato per calc. lambda in Form. 23

    double delta_f = 10000; //Banda del singolo canale

    double phases[Ndrones][Nusers][P];
    double modules[Ndrones][Nusers][P];

    double lambdav = 0;
    double alpha_BG = -4;
    int lambdaf = 1; // 0: lamda_g=0, otherwise: lambda_v
    int etaf = 1; // 0: eta=0, otherwise: eta
    int perfect_BG = 0; // 0: cos(), 1: cos()=1, -1: cos()=-1
    int normalized = 0; // 0: not normalized, otherwise yes

    double phi = 0;

    double K_BG_nu = sqrt (K_BG / (K_BG + 1));
    double K_BG_sigma = sqrt (1. / (K_BG + 1));

    //Per ogni coppia BS-RIS
    for (int d = 0; d < Ndrones; d++)
      {

        d_BR[d] = sqrt (pow (pos[d][0] - pos_BS[0], 2) + pow (pos[d][1] - pos_BS[1], 2) +
                        pow (pos[d][2] - pos_BS[2], 2)); //Distanza BS-RIS 3D

        //Paper: near Formula 2
        sin_theta_br[d] = (pos[d][2] - pos_BS[2]) / d_BR[d]; //Seno dell'angolo verticale

        double dist = sqrt (pow (pos[d][0] - pos_BS[0], 2) +
                            pow (pos[d][1] - pos_BS[1], 2)); //Distanza BS-RIS piano XY

        if (dist == 0)
          {
            cos_phi_br[d] = 0; // Il drone si trova nelle stesse cordinate xy della BS, non abbiamo quindi angolo orizzontale
            sin_phi_br[d] = 0; //
          }
        else
          {
            phi = atan2 ((pos_BS[1] - pos[d][1]), (pos_BS[0] - pos[d][0]));
            cos_phi_br[d] = cos (phi);
            sin_phi_br[d] = sin (phi);
          }

        K_BR[d] = Kmin * exp (A2 * asin (sin_theta_br[d])); //Paper: Formula 13
      }

    //Distanza BS-GU per ogni utente
    for (int u = 0; u < Nusers; u++)
      {
        d_BG[u] = sqrt (pow (pos_BS[0] - pos_UE[u][0], 2) + pow (pos_BS[1] - pos_UE[u][1], 2) +
                        pow (pos_BS[2] - pos_UE[u][2], 2));
      }

    //Per ogni coppia GU-RIS (Anche se una RIS non sta servendo quel GU)
    for (int u = 0; u < Nusers; u++)
      {
        for (int d = 0; d < Ndrones; d++)
          {
            d_RG[d][u] =
                sqrt (pow (pos[d][0] - pos_UE[u][0], 2) + pow (pos[d][1] - pos_UE[u][1], 2) +
                      pow (pos[d][2] - pos_UE[u][2], 2)); //

            if (etaf != 0)
              {
                if (normalized == 0)
                  etav[d][u] = sqrt (beta_BRG * pow (d_BR[d] * d_RG[d][u], -2));
                else
                  etav[d][u] = 1;
              }
            else
              etav[d][u] = 0;

            sin_theta_rg[d][u] = (pos[d][2] - pos_UE[u][2]) / d_RG[d][u]; // |z_U - z_G|/d_RG
            double dist =
                sqrt (pow (pos[d][0] - pos_UE[u][0], 2) + pow (pos[d][1] - pos_UE[u][1], 2));

            if (dist == 0)
              {
                cos_phi_rg[d][u] = 0;
                sin_phi_rg[d][u] = 0;
              }
            else
              {
                phi = atan2 ((pos_UE[u][1] - pos[d][1]), (pos_UE[u][0] - pos[d][0]));
                cos_phi_rg[d][u] = cos (phi);
                sin_phi_rg[d][u] = sin (phi);
              }
          }
      }


       double nu_BRG[Nusers], sig_BRG[Nusers], K[Nusers], sigma[Nusers];
    for (int u = 0; u < Nusers; u++)
      {
        nu_BRG[u] = 0;
        sig_BRG[u] = 0;
        lambdav = 0;
        if (lambdaf != 0)
          {
            if (normalized == 0)
              lambdav = sqrt (beta_BG * pow (d_BG[u], alpha_BG));
            else
              lambdav = 1;
          }

        for (int d = 0; d < Ndrones; d++)
          {
            K_RG[d][u] = Kmin * exp (A2 * asin (sin_theta_rg[d][u]));
            double K_BRG_nu = sqrt (K_BR[d] * K_RG[d][u] / ((K_BR[d] + 1) * (K_RG[d][u] + 1)));
            double K_BRG_sigma = sqrt ((K_BR[d] + K_RG[d][u]) / ((K_BR[d] + 1) * (K_RG[d][u] + 1)));
            //Per ogni patch
            for (int p = 0; p < P; p++)
              {
                modules[d][u][p] = etav[d][u] * K_BRG_nu;
                //Recupero l'indice dell'utente che quella patch sta servendo da action
                int indexu = int ((action[2 * Ndrones + d * P + p] + 1) / (2. / Nusers));

                double phi_x =
                    d_r * (sin_theta_br[d] * cos_phi_br[d] + sin_theta_rg[d][u] * cos_phi_rg[d][u] -
                           sin_theta_br[d] * cos_phi_br[d] -
                           sin_theta_rg[d][indexu] * cos_phi_rg[d][indexu]);
                double phi_y =
                    d_c * (sin_theta_br[d] * sin_phi_br[d] + sin_theta_rg[d][u] * sin_phi_rg[d][u] -
                           sin_theta_br[d] * sin_phi_br[d] -
                           sin_theta_rg[d][indexu] * sin_phi_rg[d][indexu]);

                double pigr = 2 * M_PI;
                if (modf (phi_x * M_PI * (f_c + delta_f * u) / C, &pigr) ==0) //Controllo se è multiplo di pi greco altrimenti il denominatore è 0
                  modules[d][u][p] *= (CC[p] - cc[p] + 1); //Paper: Formula 19 (Chi)
                else
                  modules[d][u][p] *=
                      sin ((CC[p] - cc[p] + 1) * phi_x * M_PI * (f_c + delta_f * u) / C) /
                      sin (phi_x * M_PI * (f_c + delta_f * u) / C); //Paper: Formula 19 (Chi)

                if (modf (phi_y * M_PI * (f_c + delta_f * u) / C, &pigr) ==
                    0) //Controllo se è multiplo di pi greco altrimenti il denominatore è 0
                  {
                    modules[d][u][p] *= (RR[p] - rr[p] + 1); //Paper: Formula 19 (Chi)
                  }
                else
                  modules[d][u][p] *=
                      sin ((RR[p] - rr[p] + 1) * phi_y * M_PI * (f_c + delta_f * u) / C) /
                      sin (phi_y * M_PI * (f_c + delta_f * u) / C); //Paper: Formula 19 (Chi)

                phases[d][u][p] =
                    -2 * M_PI * (f_c + delta_f * u) / C *
                    (d_BR[d] + d_RG[d][u] - d_BR[d] - d_RG[d][indexu]); //Paper: Formula 19 (Omega)
              }

            for (int p = 0; p < P; p++)
              {
                for (int d1 = 0; d1 <= d; d1++)
                  {
                    for (int p1 = 0; p1 < P; p1++)
                      {
                        if (d == d1)
                          {
                            if (p1 <= p)
                              p1 = p + 1;
                            if (p == (P - 1))
                              break;
                          }
                        nu_BRG[u] += 2 * abs (modules[d][u][p]) * abs (modules[d1][u][p1]) *
                                     cos (phases[d][u][p] - phases[d1][u][p1]);
                      }
                  }
                nu_BRG[u] += pow (modules[d][u][p], 2);
                if (perfect_BG == 0)
                  nu_BRG[u] += 2 * abs (modules[d][u][p]) * lambdav * K_BG_nu *
                               cos (phases[d][u][p] + 2 * M_PI * (f_c + delta_f * u) / C * d_BG[u]);
                else if (perfect_BG == 1)
                  nu_BRG[u] += 2 * abs (modules[d][u][p]) * lambdav * K_BG_nu;
                else if (perfect_BG == -1)
                  nu_BRG[u] -= 2 * abs (modules[d][u][p]) * lambdav * K_BG_nu;
                sig_BRG[u] += pow (etav[d][u], 2) * pow (K_BRG_sigma, 2) * (RR[p] - rr[p] + 1) *
                              (CC[p] - cc[p] + 1);
              }
          }
        nu_BRG[u] += pow (lambdav * K_BG_nu, 2);
        sig_BRG[u] += pow (lambdav * K_BG_sigma, 2);
        K[u] = nu_BRG[u] / sig_BRG[u];
        sigma[u] = nu_BRG[u] + sig_BRG[u];
        if (sqrt (2 * K[u]) > 2.5848)
          invpowcdff[u] = pow (sqrt (2 * K[u]) +
                                   (1. / (2 * invqfunc)) *
                                       log (sqrt (2 * K[u]) / (sqrt (2 * K[u]) - invqfunc)) -
                                   invqfunc,
                               2) /
                          (2 * (K[u] + 1) / sigma[u]);
        else
          invpowcdff[u] =
              pow (sqrt (-2 * log (1 - epsilon)) * exp (K[u] / 2), 2) / (2 * (K[u] + 1) / sigma[u]);

        rate[u] = B * log2 (1 + (.01 * invpowcdff[u] / sigma_d));
      }
  }

  IrsAssistedChannnelGain ()
  {
  }
};
