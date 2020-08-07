% Calcolo per la durata di simulazioni con lato del campo (l),altezza di
% volo (h),accelerazione (acc) e velocità  massima (vel) decise dall'utente. Il numero di
% punti della traiettoria è calcolato dalla funzione "calcolo_punti". I
% calcoli si basano sul "constant acceleration mobility model".

function [durata] = durata(n_punti,h,l,acc,vel)

% Calcolo tempo relativo al primo tratto della traiettoria

s1 = sqrt(2)*h;
azl1 = (vel^2)/(2*acc);         %azl = acceleration_zone_length [m], total length of acceleration zones
azt1 = vel/acc;               %azt = acceleration_time_length [s],  total time of acceleration zones
t1 = 2*azt1 + (s1-2*azl1)/vel;

%Calcolo la somma dei tempi per percorrere la ditanza tra due punti
s2  = 2*h;
azl2 = (vel^2)/(2*acc);
azt2 = vel/acc;
t2p = 2*azt2 + (s2-2*azl2)/vel;
t2 = (t2p)*(n_punti-3);

%Calcolo il tempo per l'ultimo tratto della traiettoria
s3 = h;
azl3 = (vel^2)/(2*acc);
azt3 = vel/acc;
t3 = 2*azt3 + (s3-2*azl3)/vel;


durata = round(t1 + t2 + t3)
return
end

