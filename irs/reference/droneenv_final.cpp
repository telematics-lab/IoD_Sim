#include <iostream>
#include <math.h>
#include <cstring>

#include <random>
#include <complex>

#include <fstream>
#include <algorithm>
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
// g++ -c -fPIC ../droneenv_final.cpp -o droneenv.o && g++ -shared -Wl,-soname,libdroneenv.so -o libdroneenv.so  droneenv.o
class DroneEnv{
    public:
        const double MAX_V = 30.0;
        const double MAX_X = 150.0;
        const double C = 299792458.0; //Velocità della luce
        const static int Nusers = 20; //Utenti
        const static int Ndrones = 3; //IRS
        const static int Nsub = 20;
        const static int P = 4; //Patch per IRS
        int rr[P] = {1,25,1,25};      //
        int RR[P] = {24,48,24,48};    // Suddivisione in patch
        int cc[P] = {1,1,25,25};      //
        int CC[P] = {24,24,48,48};    //
        double d_r = 0.01;   //Larghezza PRU (X)
        double d_c = 0.01;  //Altezza PRU (Y)
        double s = d_c * d_r;   //Area PRU
        double B = 10000;   //Larghezza di banda (10kHz)
        double sigma_d = pow(10,((-174+10*log10(B)-30)/10));  // ro^2 ovvero il rumore in mW
        int max_episodes = 10;  //Step
        double pos[Ndrones][3] = {{0,-150,50},{0,-150,50},{0,-150,50}}; //Posizione dei "Ndrones" droni
        double pos_BS[3] = {0,0,0}; //Posizione della singola Base Station

        double pos_UE[Nusers][3] = {{120,-10,0},{50,-100,0},{0,100,0},{-100,20,0},
                                    {-120,-50,0},{-90,100,0},{70,80,0},{-50,-90,0},
                                    {130,-40,0},{-90,60,0},{-150,-10,0},{-90,-120,0},
                                    {40,130,0},{20,-140,0},{-120,90,0},{-90,140,0},
                                    {-140,-100,0},{90,-90,0},{40,-110,0},{115,40,0}}; //Posizione dei "Nusers" utenti

        double d_BR[Ndrones],d_RG[Ndrones][Nusers],d_BG[Nusers],etav[Ndrones][Nusers],sin_theta_br[Ndrones], cos_phi_br[Ndrones], sin_phi_br[Ndrones],
                sin_theta_rg[Ndrones][Nusers], cos_phi_rg[Ndrones][Nusers], sin_phi_rg[Ndrones][Nusers], element_interference[Nusers],
                sin_theta_rg_probe[Ndrones][Nusers], cos_phi_rg_probe[Ndrones][Nusers], sin_phi_rg_probe[Ndrones][Nusers],
                nu_BRG[Nusers], sig_BRG[Nusers], K[Nusers], nu_BRG_max[Nusers], sig_BRG_max[Nusers], K_max[Nusers], sigma[Nusers], sigma_max[Nusers], invpowcdff[Nusers],
                invpowcdff_max[Nusers], rate[Nusers], rate_max[Nusers], data[Nusers], data_n[Nusers],penalty[Ndrones], K_BR[Ndrones], K_RG[Ndrones][Nusers];

        // double beta_0 = pow(10,-3);
        // double beta_0 = 1;
        double d_RG_probe = 0;
        double epsilon = 0.01;
        double invqfunc = 2.3263478740408408; //for eps = 0.01

        double Kmin_db = 6;
        double Kmax_db = 10;
        double Kmin = pow(10,(Kmin_db/10)); //A1
        double Kmax = pow(10,(Kmax_db/10));
        double A2 = log(pow(Kmax/Kmin,2))/M_PI;
        double K_BG = Kmin;

        double f_c = 10.5e9;
        // double beta_BRG = pow(10,-5)*s;
        double beta_BRG = pow(C/f_c,2)*s/(64*pow(M_PI,3));
        // double beta_BRG = pow(C/f_c,2)/(pow(4*M_PI,2));
        double beta_BG = pow(C/f_c,2)/(pow(4*M_PI,2));
        // double beta_BG = pow(10,-5);//
        double delta_f = 10000;
        double phases[Ndrones][Nusers][P];
        double modules[Ndrones][Nusers][P];
        double scheduling[Nusers][Nsub] = {0};
        int patches[Ndrones*P] = {0};
        int stepN = 0;
        int supstep = 0;
        double reward = 0;
        double numerator = 0;
        double denominator = 0;
        double lambdav = 0;
        double alpha_BG = -4;
        double pstep = -1;
        int count = 0;
        int lambdaf = 1; // 0: lamda_g=0, otherwise: lambda_v
        int etaf = 1; // 0: eta=0, otherwise: eta
        int perfect_BG = 0; // 0: cos(), 1: cos()=1, -1: cos()=-1
        int probe_RG = 0; // 0: no probe, otherwise: uses last two actions to probe the enviroment
        int norm_sched = 0; // 0: true, otherwise: false
        int normalized = 0; // 0: not normalized, otherwise yes

        double *min;
        double fairfactor = 8e3;
        bool threshold = true;
        DroneEnv(){
        }

        void set_lambdaf(int* v){
            lambdaf = *v;
        }

        void set_etaf(int* v){
            etaf = *v;
        }

        void set_perfect_BG(int* v){
            perfect_BG = *v;
        }

        void set_K_BG(double* v){
            if (*v == 0) K_BG = *v;
            else K_BG = Kmin;
        }

        void set_RC(int* v){
            int l = 0;
            for (int p=0;p<P;p++){
                rr[p] = v[p*l];
                RR[p] = v[p*l+1];
                cc[p] = v[p*l+2];
                CC[p] = v[p*l+3];
                l += 4;
            }

        }

        void set_probe_RG(int* v){
            probe_RG = *v;
        }

        void set_normalized(int* v){
            normalized = *v;
        }

        void set_epsilon(double* v){
            epsilon = *v;
        }

        void set_invqfunc(double* v){
            invqfunc = *v;
        }

        void set_norm_sched(int* v){
            norm_sched = *v;
        }

        void set_fairfactor(double* v){
            fairfactor = *v;
        }

        void get_UE(double* ret){
            for (int u=0; u<Nusers; u++) {
                ret[3*u] = pos_UE[u][0]/MAX_X;
                ret[3*u+1] = pos_UE[u][1]/MAX_X;
                ret[3*u+2] = pos_UE[u][2]/MAX_X;
            }
        }

        void get_invpowcdff(double* ret){
            for (int u=0; u<Nusers; u++) ret[u] =  invpowcdff[u];
        }

        void get_BS(double* ret){
            ret[0] = pos_BS[0]/MAX_X;
            ret[1] = pos_BS[1]/MAX_X;
            ret[2] = pos_BS[2]/MAX_X;
        }

        void get_max_episodes(int* ret){
            ret[0] = max_episodes;
        }

        void get_ndrones(int* ret){
            ret[0] = Ndrones;
        }

        void get_nGUs(int* ret){
            ret[0] = Nusers;
        }

        void get_npatches(int* ret){
            ret[0] = P;
        }

        void get_nsubs(int* ret){
            ret[0] = Nsub;
        }

        void get_nstep(int* ret){
            ret[0] = stepN;
        }

        void get_nu(double* ret){
            for (int u=0; u<Nusers; u++) ret[u] = nu_BRG[u];
        }

        void get_sigma(double* ret){
            for (int u=0; u<Nusers; u++) ret[u] = sig_BRG[u];
        }

        void get_eta(double* ret){
            ret[0] = etav[0][0];
        }

        void get_lambda(double* ret){
            ret[0] = lambdav;
        }

        void get_pos(double* ret){
            for (int d=0; d<Ndrones; d++) {
                ret[2*d] = pos[d][0]/MAX_X;
                ret[2*d+1] = pos[d][1]/MAX_X;
                // ret[3*d+2] = pos[d][2];
            }
        }

        void step(double* action, double* ret){
            pstep = stepN;
            stepN+=1;
            supstep++;
            numerator = 0;
            denominator = 0;
            count = 0;
            double phi = 0;
            double penaltyS = 1;
            std::cout.precision(18);

            double K_BG_nu = sqrt(K_BG/(K_BG+1));
            double K_BG_sigma = sqrt(1./(K_BG+1));

            for (int d=0;d < Ndrones;d++){
                if (probe_RG == 0){
                    pos[d][0] += action[2*d]*MAX_V;
                    pos[d][1] += action[2*d+1]*MAX_V;
                } else {
                    pos[d][0] = action[2*d]*MAX_X;
                    pos[d][1] = action[2*d+1]*MAX_X;
                }

                //returning normalized drones' positions
                ret[3*d] = pos[d][0]/MAX_X;
                ret[3*d+1] = pos[d][1]/MAX_X;
                ret[3*d+2] = pos[d][2]/MAX_X;

                d_BR[d] = sqrt(pow(pos[d][0] - pos_BS[0],2) + pow(pos[d][1] - pos_BS[1],2) + pow(pos[d][2] - pos_BS[2],2));
                sin_theta_br[d] = (pos[d][2]-pos_BS[2])/d_BR[d];
                double dist = sqrt(pow(pos[d][0] - pos_BS[0],2) + pow(pos[d][1] - pos_BS[1],2));
                    if (dist == 0){
                        cos_phi_br[d] = 0;
                        sin_phi_br[d] = 0;
                    } else {
                        phi = atan2((pos_BS[1]-pos[d][1]),(pos_BS[0]-pos[d][0]));
                        cos_phi_br[d] = cos(phi);
                        sin_phi_br[d] = sin(phi);
                    }
                K_BR[d] = Kmin*exp(A2*asin(sin_theta_br[d]));
            }
            for (int u=0; u<Nusers; u++) {
                d_BG[u] = sqrt(pow(pos_BS[0] - pos_UE[u][0],2) + pow(pos_BS[1] - pos_UE[u][1],2) + pow(pos_BS[2] - pos_UE[u][2],2));

                //returning normalized GUs' positions
                ret[3*Ndrones+3*u] = pos_UE[u][0]/MAX_X;
                ret[3*Ndrones+3*u+1] = pos_UE[u][1]/MAX_X;
                ret[3*Ndrones+3*u+2] = pos_UE[u][2]/MAX_X;
            }
            for (int u=0; u<Nusers; u++) {
                for (int d=0;d < Ndrones;d++){
                    cos_phi_rg_probe[d][u] = 0;
                    sin_phi_rg_probe[d][u] = 0;
                    d_RG[d][u] = sqrt(pow(pos[d][0] - pos_UE[u][0],2) + pow(pos[d][1] - pos_UE[u][1],2) + pow(pos[d][2] - pos_UE[u][2],2));
                    d_RG_probe = sqrt(pow(pos[d][0] - action[2*Ndrones+Ndrones*P],2) + pow(pos[d][1] - action[2*Ndrones+Ndrones*P+1],2)+ pow(pos[d][2] - 0,2));
                    if (etaf !=0 ) {if (normalized == 0) etav[d][u] = sqrt(beta_BRG*pow(d_BR[d]*d_RG[d][u],-2)); else etav[d][u] = 1;} else etav[d][u] = 0;
                    sin_theta_rg[d][u] = (pos[d][2] - pos_UE[u][2])/d_RG[d][u]; // |z_U - z_G|/d_RG
                    sin_theta_rg_probe[d][u] = (pos[d][2] - 0)/d_RG_probe; // |z_U - z_G|/d_RG
                    double dist = sqrt(pow(pos[d][0] - pos_UE[u][0],2) + pow(pos[d][1] - pos_UE[u][1],2));
                    double dist_probe = sqrt(pow(pos[d][0] - action[2*Ndrones+Ndrones*P],2) + pow(pos[d][1] - action[2*Ndrones+Ndrones*P+1],2));
                    if (dist == 0){
                        cos_phi_rg[d][u] = 0;
                        sin_phi_rg[d][u] = 0;
                    } else {
                        phi = atan2((pos_UE[u][1]-pos[d][1]),(pos_UE[u][0]-pos[d][0]));
                        cos_phi_rg[d][u] = cos(phi);
                        sin_phi_rg[d][u] = sin(phi);
                    }
                    if (dist_probe != 0){
                        phi = atan2((action[2*Ndrones+Ndrones*P+1]-pos[d][1]), (action[2*Ndrones+Ndrones*P]-pos[d][0]));
                        cos_phi_rg_probe[d][u] = cos(phi);
                        sin_phi_rg_probe[d][u] = sin(phi);
                       }

                }
            }
            for (int u=0; u<Nusers; u++) {
                nu_BRG[u] = 0;
                sig_BRG[u] = 0;
                nu_BRG_max[u] = 0;
                sigma_max[u] = 0;
                lambdav = 0;
                if (lambdaf != 0) {if (normalized == 0) lambdav = sqrt(beta_BG*pow(d_BG[u],alpha_BG)); else lambdav = 1;}
                for (int d=0;d < Ndrones;d++){
                    K_RG[d][u] = Kmin*exp(A2*asin(sin_theta_rg[d][u]));
                    double K_BRG_nu = sqrt(K_BR[d]*K_RG[d][u]/((K_BR[d]+1)*(K_RG[d][u]+1)));
                    double K_BRG_sigma = sqrt((K_BR[d]+K_RG[d][u])/((K_BR[d]+1)*(K_RG[d][u]+1)));
                    for (int p=0; p < P; p++){
                        modules[d][u][p] = etav[d][u]*K_BRG_nu;
                        //scheduling patch -1<=x<=1
                        int indexu = int ((action[2*Ndrones+d*P+p]+1)/(2./Nusers));
                        if (indexu == Nusers) indexu = Nusers-1;
                        //scheduling patch x \in N
                        if (norm_sched != 0) indexu = int (action[2*Ndrones+d*P+p]);
                        //returning patch-GU association
                        ret[3*(Ndrones+Nusers)+2+Nusers+d*P+p] = indexu;

                        double phi_x = d_r*(sin_theta_br[d]*cos_phi_br[d]+sin_theta_rg[d][u]*cos_phi_rg[d][u]
                        -sin_theta_br[d]*cos_phi_br[d]-sin_theta_rg[d][indexu]*cos_phi_rg[d][indexu]);
                        double phi_y = d_c*(sin_theta_br[d]*sin_phi_br[d]+sin_theta_rg[d][u]*sin_phi_rg[d][u]
                        -sin_theta_br[d]*sin_phi_br[d]-sin_theta_rg[d][indexu]*sin_phi_rg[d][indexu]);

                        if (probe_RG != 0){
                            phi_x = d_r*(sin_theta_rg_probe[d][u]*cos_phi_rg_probe[d][u]-sin_theta_rg[d][indexu]*cos_phi_rg[d][indexu]);
                            phi_y = d_c*(sin_theta_rg_probe[d][u]*sin_phi_rg_probe[d][u]-sin_theta_rg[d][indexu]*sin_phi_rg[d][indexu]);
                        }

                        double pigr = 2*M_PI;
                        if (modf(phi_x*M_PI*(f_c+delta_f*u)/C,&pigr) == 0) modules[d][u][p] *= (CC[p]-cc[p]+1);
                        else modules[d][u][p] *= sin((CC[p]-cc[p]+1)*phi_x*M_PI*(f_c+delta_f*u)/C)/sin(phi_x*M_PI*(f_c+delta_f*u)/C);
                        if (modf(phi_y*M_PI*(f_c+delta_f*u)/C,&pigr) == 0) {modules[d][u][p] *= (RR[p]-rr[p]+1);}
                        else modules[d][u][p] *= sin((RR[p]-rr[p]+1)*phi_y*M_PI*(f_c+delta_f*u)/C)/sin(phi_y*M_PI*(f_c+delta_f*u)/C);
                        phases[d][u][p] = -2*M_PI*(f_c+delta_f*u)/C*(d_BR[d]+d_RG[d][u]-d_BR[d]-d_RG[d][indexu]);
                        if (action[2*Ndrones+Ndrones*P] == 0 && action[2*Ndrones+Ndrones*P+1] == 70) {
                            double phiphi = (atan2(-(action[2*Ndrones+Ndrones*P+1]-pos[d][1]), -(action[2*Ndrones+Ndrones*P]-pos[d][0])));
                            std::cout<<"mod "<<modules[d][u][p]<< " phi : "<<phiphi<<" cos "<<cos(phiphi)<<" sin "<<sin(phiphi)<<" d_rg "<<d_RG[d][0]<<(CC[p]-cc[p]+1)<<std::endl;
                            std::cout<<d_BR[d]+10*sin_theta_br[d]*cos_phi_br[d]<<std::endl;//-10*sin_theta_rg[d][u]*sin_phi_rg[d][u]
                            std::cout<<d_BR[d]+10*sin_theta_br[d]*cos_phi_br[d]-10*sin_theta_br[d]*sin_phi_br[d]<<std::endl;
                        }
                    }
                    for (int p=0; p < P; p++){
                        for (int d1=0;d1 <= d;d1++){
                            for (int p1 = 0;p1 < P; p1++){
                                if (d==d1){
                                    if (p1<=p) p1 = p+1;
                                    if (p==(P-1)) break;
                                }
                                nu_BRG[u] += 2*abs(modules[d][u][p])*abs(modules[d1][u][p1])*cos(phases[d][u][p] - phases[d1][u][p1]);
                                count++;
                            }
                        }
                        nu_BRG[u] += pow(modules[d][u][p],2);
                        if (perfect_BG == 0) nu_BRG[u] += 2*abs(modules[d][u][p])*lambdav*K_BG_nu*cos(phases[d][u][p] + 2*M_PI*(f_c+delta_f*u)/C*d_BG[u]);
                        else if (perfect_BG == 1) nu_BRG[u] += 2*abs(modules[d][u][p])*lambdav*K_BG_nu;
                        else if (perfect_BG == -1) nu_BRG[u] -= 2*abs(modules[d][u][p])*lambdav*K_BG_nu;
                        sig_BRG[u] += pow(etav[d][u],2)*pow(K_BRG_sigma,2)*(RR[p]-rr[p]+1)*(CC[p]-cc[p]+1);
                    }
                }
                nu_BRG[u] += pow(lambdav*K_BG_nu,2);
                sig_BRG[u] += pow(lambdav*K_BG_sigma,2);
                K[u] = nu_BRG[u]/sig_BRG[u];
                sigma[u] = nu_BRG[u]+sig_BRG[u];
                if (sqrt(2*K[u]) > 2.5848)
                invpowcdff[u] = pow(sqrt(2*K[u]) + (1./(2*invqfunc))*log(sqrt(2*K[u])/(sqrt(2*K[u])-invqfunc)) -invqfunc,2)/(2*(K[u]+1)/sigma[u]);
                else
                invpowcdff[u] = pow(sqrt(-2*log(1-epsilon))*exp(K[u]/2),2)/(2*(K[u]+1)/sigma[u]);

                rate[u] = B*log2(1+(.01*invpowcdff[u]/sigma_d));
                data[u] += rate[u]/B;
                data_n[u] += rate[u]/B;
                for (int h=0; h < u; h++){
                    denominator += abs(data[u]-data[h])/fairfactor;
                }
                numerator += data[u];
                if (probe_RG != 0) break;
            }









            reward = numerator/(1+denominator)/1e3;
            ret[3*(Ndrones+Nusers)] = reward;
            //returning done
            if (stepN >= max_episodes) {
                ret[3*(Ndrones+Nusers)+1] = 1.;
            }
            else {
                ret[3*(Ndrones+Nusers)+1] = 0.;
            }

            if ((stepN> 1 && sqrt(pow(pos[0][0]-pos[1][0],2)+pow(pos[0][1]-pos[1][1],2)+pow(pos[0][2]-pos[1][2],2)) <= 10)) ret[3*(Ndrones+Nusers)+1] = 1;
            if ((stepN> 1 && sqrt(pow(pos[0][0]-pos[2][0],2)+pow(pos[0][1]-pos[2][1],2)+pow(pos[0][2]-pos[2][2],2)) <= 10)) ret[3*(Ndrones+Nusers)+1] = 1;
            if ((stepN> 1 && sqrt(pow(pos[1][0]-pos[2][0],2)+pow(pos[1][1]-pos[2][1],2)+pow(pos[1][2]-pos[2][2],2)) <= 10)) ret[3*(Ndrones+Nusers)+1] = 1;
            //returning GUs' data amounts
            for (int u=0; u<Nusers; u++) ret[3*(Ndrones+Nusers)+2+u] = data[u];
        }





};
