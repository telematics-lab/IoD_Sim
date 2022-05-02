%IRS dimension where M is the number of columns and N is the number of rows
M=30;   
N=30;

%Dimension (in meters) of a PRU 
x_Side=0.5; 
y_Side=0.5;

%Angles used for rotation, theta refers to 
%X-axis rotation, phi refers to Z-axis rotation 
theta=pi/4;
phi=pi/4;

%MonilityModel coordinate
MMCoordinate=[20,20,20];

figure('Name','IRS rotation and shift','NumberTitle','off','Position', [10 10 900 900]);
for m = 1:M
   for n = 1:N
   P=Calc_2D_Coordinate(m,n,M,N,x_Side,y_Side); %This function calculates the coordinates of the (m,n)-th PRU center. 
   plot3(P(1),P(2),P(3),'.','Color','red');
   hold on
   PR=Rotate(Rotate(P,'x',theta),'z',phi); %This function rotates a point around a given axis.
   plot3(PR(1),PR(2),PR(3),'.','Color','green');
   hold on
   PS=Shift(PR,MMCoordinate); %This function shifts a point in the 3D-space by a given shift vector.
   plot3(PS(1),PS(2),PS(3),'.','Color','blue');
   hold on
   end
end
axis([-50 50 -50 50 -50 50])
xlabel('x') 
ylabel('y') 
zlabel('z') 
hold off