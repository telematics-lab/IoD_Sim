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
tbl = readtable("C:\Users\franc\OneDrive - Politecnico di Bari\Desktop\Tesi\Tesi LaTex\DlRsrpSinrStats.txt", opts);
load('C:\Users\franc\OneDrive - Politecnico di Bari\Desktop\Tesi\Tesi LaTex\nTxPDU.mat');
load('C:\Users\franc\OneDrive - Politecnico di Bari\Desktop\Tesi\Tesi LaTex\nRxPDU.mat');
load('C:\Users\franc\OneDrive - Politecnico di Bari\Desktop\Tesi\Tesi LaTex\TBS.mat');

%% Convert to output type
time = tbl.time;
rsrp = tbl.rsrp;
sinr = tbl.sinr;
nTxPDUs =nTxPDUs(1:length(nTxPDUs));
nRxPDUs = nRxPDUs(1:length(nRxPDUs));
TBs = sizeTb1(1:length(sizeTb1));

time = time(~isnan(time));
rsrp = rsrp(~isnan(rsrp));
sinr = sinr(~isnan(sinr));
TBs = TBs(~isnan(TBs));
nTxPDUs = nTxPDUs(~isnan(nTxPDUs));
nRxPDUs = nRxPDUs(~isnan(nRxPDUs));

for p = 1:length(nTxPDUs)
    pdr(p) = (nRxPDUs(p)./nTxPDUs(p)).*100;
end

for o = 1:length(TBs)    
    TBs(o) = (TBs(o)*8)/1000 ; % Byte to Mbps conversion
end

sinr(9:20768)=[]; 
rsrp(9:20768)=[]; 

for i=1:length(sinr)
    
    rsrp(i) = 10 * log10(mean(1000*rsrp(i))); % Watt to dBm conversion
    sinr(i) = 10 * log10(mean(sinr(i)));   
    
end
l=1;
for m = 1:5737  
    
    TBs1(m) =  mean(TBs( l : l + 99)); %Transport Block size
    l = l + 99;
    
end
TBs1(5738) = mean(TBs( l : l + 5178));
TBs1 = TBs1';

figure(1);
%title ('Throughput','Fontsize',20)
plot(sinr,TBs,'b')
set(gca,'FontSize',20)
xlabel('SINR (dB)','Fontsize', 20)
ylabel('DL Throughput [Mbps]','Fontsize', 20)

% figure(2);
% title ('Throughput','Fontsize',20)
% plot(pdr,TBs1,'b','LineWidth',1.2)
% grid on
% set(gca,'FontSize',20)
% xlabel('PDR [%]','Fontsize', 20)
% ylabel('DL Throughput [Mbps]','Fontsize', 20)

% figure(3);
% XX=meshgrid(rsrp);
% YY=meshgrid(sinr);
% ZZ=meshgrid(TBs);
% mesh(XX,YY,ZZ,'Facelighting','gouraud','LineWidth',5);
% ylabel('SINR (dB)','Fontsize', 20)
% xlabel('RSRP (dBm)','Fontsize', 20)
% set(gca,'FontSize',20)
% zlabel('DL Throughput [Mbps]','Fontsize', 20)
% colormap jet
% colorbar


%% Clear temporary variables
clear opts tbl