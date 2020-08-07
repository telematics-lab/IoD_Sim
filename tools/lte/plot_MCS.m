%% Setup the Import Options
clear all;
clc;
opts = delimitedTextImportOptions("NumVariables", 11);

% Specify range and delimiter
opts.DataLines = [1, Inf];
opts.Delimiter = "\t";

% Specify column names and types
opts.VariableNames = ["time", "Var2", "Var3", "Var4", "Var5", "Var6", "mcsTb1", "Var8", "Var9", "Var10", "Var11"];
opts.SelectedVariableNames = ["time", "mcsTb1"];
opts.VariableTypes = ["double", "string", "string", "string", "string", "string", "double", "string", "string", "string", "string"];
opts = setvaropts(opts, [2, 3, 4, 5, 6, 8, 9, 10, 11], "WhitespaceRule", "preserve");
opts = setvaropts(opts, [2, 3, 4, 5, 6, 8, 9, 10, 11], "EmptyFieldRule", "auto");
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Import the data
tbl = readtable("C:\Users\franc\OneDrive - Politecnico di Bari\Desktop\Tesi\Tesi LaTex\DlMacStats.txt", opts);

%% Convert to output type
time = tbl.time;
mcsTb1 = tbl.mcsTb1;

plot(time,mcsTb1,'-b')
% title('Qualità  della rete','Fontsize', 20),
set(gca,'FontSize',20)
xlabel('Time [s]','Fontsize', 20)
ylabel('MCS index','Fontsize', 20)
hold on

%% Clear temporary variables
clear opts tbl
