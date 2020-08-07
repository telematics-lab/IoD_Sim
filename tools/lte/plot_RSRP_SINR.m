%% Setup the Import Options
clc;
clear all;
opts = delimitedTextImportOptions("NumVariables", 7);

% Specify range and delimiter
opts.DataLines = [1, Inf];
opts.Delimiter = "\t";

% Specify column names and types
opts.VariableNames = ["time", "Var2", "Var3", "Var4", "rsrp", "sinr", "Var7"];
opts.SelectedVariableNames = ["time", "rsrp", "sinr"];
opts.VariableTypes = ["double", "string", "string", "string", "double", "double", "string"];
opts = setvaropts(opts, [2, 3, 4, 7], "WhitespaceRule", "preserve");
opts = setvaropts(opts, [2, 3, 4, 7], "EmptyFieldRule", "auto");
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Import the data
tbl = readtable("C:\Users\Piero\OneDrive - Politecnico di Bari\Tesi\2019\L3\Petruzzellis\Codici_PB\Output Scenario\DlRsrpSinrStats.txt", opts);
load('C:\Users\Piero\OneDrive - Politecnico di Bari\Tesi\2019\L3\Petruzzellis\Codici_PB\Sorgenti Matlab\punti.mat');
load('C:\Users\Piero\OneDrive - Politecnico di Bari\Tesi\2019\L3\Petruzzellis\Codici_PB\Sorgenti Matlab\TBS.mat');
%% Convert to output type
time = tbl.time;
rsrp = tbl.rsrp;
sinr = tbl.sinr;

TBs = sizeTb1(1:length(sizeTb1));
TBs = TBs(~isnan(TBs));

for i = 1:length(TBs)    
    TBs(i) = (TBs(i)*8)/1000 ; % Byte to Mbps conversion
end

for i=1:length(rsrp)
    y1(i) = 10 * log10(mean(1000*rsrp(i))); % Watt to dBm conversion
    y2(i) = 10 * log10(mean(sinr(i)));   
end
time = time(~isnan(time));
y1 = y1(~isnan(y1));
y2 = y2(~isnan(y2));
rsrp = rsrp(~isnan(rsrp));
sinr = sinr(~isnan(sinr));

figure (1);
plot(time,y1,'b')
xlabel('Time [s]','Fontsize', 20)
ylabel('RSRP [dBm]','Fontsize', 20)
set(gca,'FontSize',20)
%title ('Reference Signal Received Power (RSRP)','Fontsize', 20)

figure (2);
bar(time,y2,'b')
xlabel('Time [s]','Fontsize', 20)
ylabel('SINR [dB]','Fontsize', 20)
set(gca,'FontSize',20)
%title ('Signal-to-Interference-plus-Noise Ratio (SINR)','Fontsize', 20)

figure(3);
plot(y2,y1,'b')
set(gca,'FontSize',20)
xlabel('SINR (dB)','Fontsize', 20)
ylabel('RSRP (dBm)','Fontsize', 20)
%title ('UE Measurements','Fontsize',20)

X = punti(2:length(punti)-1,1); 
Y = punti(2:length(punti)-1,2);
time_window_2=20655; % indice della seconda time_window
j = time_window_2;
time_window = 23333; % dimensione della time_window

for i = 1:length(punti)-3
    Z(i)    = mean(y1( j : j + time_window)); %rsrp
    Z1(i)   = mean(y2( j : j + time_window));%sinr
    j = j + time_window;
end
Z(length(Z)+1)=mean(y1( j : j + length(time)-i*time_window-time_window_2));
Z1(length(Z1)+1)=mean(y2( j : j + length(time)-i*time_window-time_window_2));

l=322; % fine della prima time_window -> MAC Stats
% si vede perchè nel file di log il valore alle 322 riga è pari a
% time_window_2
time_window_3 = 23278;
for m = 1:length(punti)-3
    % 23278 è il numero che mappa la time_window nel file delle statistiche
    % di livello 2.
    y3(m) =  mean(TBs( l : l + time_window_3)); %Transport Block size
    l = l + time_window_3;
    % 23278 + 322 = 23600. Nel file di log di livello 2, il valore alla
    % 23600sima riga è 43.987.
    % Da riga 23601, inizia la nuova finestra temporale.
    
end
y3(length(y3)+1) = mean(TBs( l : l + length(TBs)-m*time_window_3-322));

figure(4);
XX=meshgrid(X);
YY=meshgrid(Y);
ZZ=meshgrid(Z);
mesh(XX,YY,ZZ,'Facelighting','gouraud','LineWidth',5);
xlabel('x [m]','FontSize',20)
ylabel('y [m]','FontSize',20)
set(gca,'FontSize',20)
zlabel('Potenza [dBm] ','FontSize',20)
colormap jet
colorbar('FontSize',16)
% ix = find(imregionalmax(ZZ));  %per vedere i max
% hold on
% plot3(XX(ix),YY(ix),ZZ(ix),'r*','MarkerSize',20)

%% Clear temporary variables
clear opts tbl