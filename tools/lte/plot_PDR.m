%% Setup the Import Options
clc;
clear all;
opts = delimitedTextImportOptions("NumVariables", 18);

% Specify range and delimiter
opts.DataLines = [1, Inf];
opts.Delimiter = "\t";

% Specify column names and types
opts.VariableNames = ["start", "Var2", "Var3", "Var4", "Var5", "Var6", "nTxPDUs", "TxBytes", "nRxPDUs", "RxBytes", "Var11", "Var12", "Var13", "Var14", "Var15", "Var16", "Var17", "Var18"];
opts.SelectedVariableNames = ["start", "nTxPDUs", "TxBytes", "nRxPDUs", "RxBytes"];
opts.VariableTypes = ["double", "string", "string", "string", "string", "string", "double", "double", "double", "double", "string", "string", "string", "string", "string", "string", "string", "string"];
opts = setvaropts(opts, [2, 3, 4, 5, 6, 11, 12, 13, 14, 15, 16, 17, 18], "WhitespaceRule", "preserve");
opts = setvaropts(opts, [2, 3, 4, 5, 6, 11, 12, 13, 14, 15, 16, 17, 18], "EmptyFieldRule", "auto");
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Import the data
tbl = readtable("C:\Users\Piero\OneDrive - Politecnico di Bari\Tesi\2019\L3\Petruzzellis\Codici\Output Scenario\DlRlcStats.txt", opts);
load('C:\Users\Piero\OneDrive - Politecnico di Bari\Tesi\2019\L3\Petruzzellis\Codici\Sorgenti Matlab\punti.mat');
load('C:\Users\Piero\OneDrive - Politecnico di Bari\Tesi\2019\L3\Petruzzellis\Codici\Sorgenti Matlab\tabella.mat');

%% Elaborations and Plots
start = tbl.start;
nTxPDUs = tbl.nTxPDUs;
TxBytes = tbl.TxBytes;
nRxPDUs = tbl.nRxPDUs;
RxBytes = tbl.RxBytes;

% Inizio primo tratto dopo la messa in quota
l = 6; % verifica l'istante temporale in maniera concorde a quanto vista prima
m = 6; % verifica l'istante temporale in maniera concorde a quanto vista prima

% Estrazione coordinate punti della traiettoria in vettori singoli
xp = punti(2:26,1); 
yp = punti(2:26,2);
zp = punti(2:26,3);

% Posizione base station
%% da parametrizzare
zsp = [500 , 500 , 0];

%%
for i = 1:25  % perchè considero 25 punti, trascurando quello iniziale e finale
    
    % Vettore distanze dei singoli punti della traiettoria dallo zsp [m]
    distance(i) = sqrt( (xp(i) - zsp(1))^2 + (yp(i)-zsp(2))^2 ); 
    
    tot_tx_pdu_p(i) = sum(nTxPDUs(m : m + 10));  
    tot_rx_pdu_p(i) = sum(nRxPDUs(m : m + 10));
   
    u(i) = sum(TxBytes(m : m + 10));
    v(i) = sum(RxBytes(m : m + 10));
    vv(i) = (v(i)./1000000).*8;  
    % Finestre di trasmissione in ciascun punto in cui staziona il drone   
    window_punti(i) = start(m);
    
    m = m + 234; % dimensione della finestra temporale
    
end

% Packet Delivery Ratio in funzione delle distanze
pdr_m = (tot_rx_pdu_p./tot_tx_pdu_p).*100;
plr = (1 - (tot_rx_pdu_p./tot_tx_pdu_p)).*100;
for k = 1:24    % perchè le finestre di trasmissione sono 24 tra ciascuna coppia di punti 
    
    tot_tx_pdu(k) = sum(nTxPDUs(l : l + 234));
    tot_rx_pdu(k) = sum(nRxPDUs(l : l + 234));
%   p(k) = sum(TxBytes(l : l + 234));
%   q(k) = sum(RxBytes(l : l + 234));
    
%   estremi di ciascuna finestra di trasmissione
    window(k) = start(l);
    
    l = l + 234;
    
    window(k+1) = start(l);
    
end

% finestre = {'finestra 1','finestra 2','finestra 3','finestra 4','finestra 5','finestra 6','finestra 7',...
%     'finestra 8','finestra 9','finestra 10','finestra 11','finestra 12','finestra 13','finestra 14',...
%     'finestra 15','finestra 16','finestra 17','finestra 18','finestra 19','finestra 20','finestra 21',...
%     'finestra 22','finestra 23','finestra 24'};
finestre = {'1',' 2','3','4','5','6',' 7',...
    ' 8',' 9',' 10',' 11',' 12',' 13',' 14',...
    ' 15',' 16',' 17',' 18',' 19',' 20',' 21',...
    ' 22',' 23',' 24'};
f = categorical(finestre,finestre);
t_start = seconds(20.6):seconds(23.4):seconds(558.8);
t_end = seconds(44.0):seconds(23.4):seconds(582.2);
finestre_Tx = f';
inizio_finestra = t_start';
termine_finestra = t_end';
Tabella_finestre = table(finestre_Tx,inizio_finestra,termine_finestra);

% Packet Delivery Ratio in funzione del tempo
pdr_t = (tot_rx_pdu./tot_tx_pdu).*100;

% primo plot
figure1 = figure;
axes1 = axes('Parent',figure1);
%bar(f,pdr_t,0.5,'FaceColor',[0.2 0.2 .6])
bar(pdr_t,0.5,'FaceColor',[0.2 0.2 .6])
xticklabels(finestre)         % This will set labels to be used for each tick of the x-axis
xticks(1:1:length(finestre))  % This will set how many ticks you want on the x-axis. 
% xtickangle(45)  
ylim ([95 100]);
box(axes1,'off');
%ytickformat('percentage');
%title('Throughput','Fontsize', 20);
set(gca,'FontSize',20)
xlabel('Finestre di Trasmissione nel tempo','Fontsize', 20)
ylabel('PDR [%] ','Fontsize', 20)
legend({'Data from Drone to BS'},'Location','southwest','Fontsize', 20)

%secondo plot
figure2 = figure;
axes2 = axes('Parent',figure2);
plot(distance,pdr_m,'b')  
xlim([0 max(distance)]);
box(axes2,'off');
xticks(0:50:max(distance));
%ytickformat('percentage');
%xtickformat('%.1f');
%title('Throughput','Fontsize', 20);
set(gca,'FontSize',20)
xlabel('Distanza dallo ZSP [m]','Fontsize', 20)
ylabel('PDR [%]','Fontsize', 20)
legend({'Data from Drone to BS'},'Location','southwest','Fontsize', 20)

% %terzo plot
figure3 = figure;
XX=meshgrid(xp);
YY=meshgrid(yp);
ZZ=meshgrid(pdr_m);
mesh(XX,YY,ZZ,'FaceLighting','gouraud','EdgeColor','interp','LineWidth',5);
colormap jet
colorbar('FontSize',16)
%title('Throughput','Fontsize', 20);
set(gca,'FontSize',20)
xlabel('x [m]','FontSize',20)
ylabel('y [m]','FontSize',20)
zlabel('PDR [%] ','Fontsize', 20)
%ztickformat('percentage');

%% Clear temporary variables
clear opts tbl