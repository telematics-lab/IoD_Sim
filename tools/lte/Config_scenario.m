%Scrive il file json di configurazione per lo scenario "lena-simple-davinci" ed
%effettua il plot della traiettoria generata da calcolo_punti
clc;
clear all;
[punti,zsp,lato,high] = calcolo_punti;
a = input ('inserire accelerazione drone: ');
v = input ('inserire velocità  massima: ');
durata = durata(size(punti,1),high,lato,a,v);
fid = fopen('C:\Users\franc\OneDrive - Politecnico di Bari\Desktop\Tesi\Tesi LaTex\config_scenarioLte.json','w');
fprintf(fid,'%s \n','{');
fprintf(fid,'%s \n','    "logOnFile": true,');
fprintf(fid,'%s \n','    "phyMode": "DsssRate11Mbps",');
fprintf(fid,'%s','    "duration": ');
fprintf(fid,'%6.1f', durata);
fprintf(fid,'%s \n',',');
fprintf(fid,'%s \n','    "drones": [');
fprintf(fid,'%s \n','        {');
fprintf(fid,'%s ','            "acceleration": ');
fprintf(fid,'%6.1f', a);
fprintf(fid,'%s \n',',');
fprintf(fid,'%s ','            "maxSpeed": ');
fprintf(fid,'%6.1f', v);
fprintf(fid,'%s \n',',');
fprintf(fid,'%s \n','            "applicationStartTime": 1.0,');
fprintf(fid,'%s ','            "applicationStopTime": ');
fprintf(fid,'%6.1f', durata);
fprintf(fid,'%s \n',',');
fprintf(fid,'%s \n','            "trajectory": [');
fprintf(fid,'%s \n','                {');
fprintf(fid,'%s \n','                    "position": [0.0, 0.0, 0.0],');
fprintf(fid,'%s \n','                    "interest": 0,');
fprintf(fid,'%s \n','                    "restTime": 1.0');
fprintf(fid,'%s \n','                },');
for i=2:(size(punti,1)-1)
    fprintf(fid,'%s \n','                {');
    fprintf(fid,'%s','                    "position": ');
    S = jsonencode(punti(i,:));
    s = strrep (S,',','.0, ');
    s = strrep(s,']','.0]');
    fprintf(fid,'%s',s);
    fprintf(fid,'%s \n',',');
    fprintf(fid,'%s \n','                    "interest": 0,');
    fprintf(fid,'%s \n','                    "restTime": 1.0');
    fprintf(fid,'%s \n','                },');
end
i=i+1;
fprintf(fid,'%s \n','                {');
fprintf(fid,'%s' ,'                    "position": ');
S = jsonencode(punti(i,:));
s = strrep (S,',','.0, ');
s = strrep(s,']','.0]');
fprintf(fid,'%s',s);
fprintf(fid,'%s \n',',');
fprintf(fid,'%s \n','                    "interest": 0,');
fprintf(fid,'%s \n','                    "restTime": 1.0');
fprintf(fid,'%s \n','                }');
fprintf(fid,'%s \n','            ]');
fprintf(fid,'%s \n','        }');
fprintf(fid,'%s \n','    ],');
fprintf(fid,'%s \n','    "ZSPs": [');
fprintf(fid,'%s \n','        {');
fprintf(fid,'%s','            "position": ');
z = jsonencode(zsp);
Z = strrep(z,',','.0, ');
Z = strrep(Z,']','.0]');
fprintf(fid,'%s \n',Z);
fprintf(fid,'%s \n','        }');
fprintf(fid,'%s \n','    ],');
fprintf(fid,'%s \n','    "logComponents": [');
fprintf(fid,'%s \n','        "Curve",');
fprintf(fid,'%s \n','        "ConstantAccelerationFlight",');
fprintf(fid,'%s \n','        "Planner",');
fprintf(fid,'%s \n','        "ConstantAccelerationDroneMobilityModel",');
fprintf(fid,'%s \n','        "DroneServer",');
fprintf(fid,'%s \n','        "DroneClient",');
fprintf(fid,'%s \n','        "ScenarioConfigurationHelper"');
fprintf(fid,'%s \n','    ]');
fprintf(fid,'%s \n','}');
fclose(fid);
X = punti(:,1);
Y = punti(:,2);
Z = punti(:,3);
plot3(X,Y,Z,'-o','Color','b','MarkerSize',3,'MarkerFaceColor','r');
set(gca,'FontSize',20)
xlabel('x [m]','FontSize',20)
ylabel('y [m]','FontSize',20)
zlabel('z [m]','FontSize',20)
high=punti(2,3);
axis([0 lato 0 lato 0 high+high/2]);
%title('Traiettoria di volo del drone.','FontSize',20)
grid
hold on
xz=lato/2;
yz=lato/2;
zz=0;
plot3(xz,yz,zz,'*','Color','r','MarkerSize',7);
legend('Traiettoria','eNB','FontSize',20);