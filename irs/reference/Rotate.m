function PR = Rotate(P,axis,angle)
X=[1 0 0; 0 cos(angle) -sin(angle); 0 sin(angle) cos(angle)];
Y=[cos(angle) 0 sin(angle); 0 1 0; -sin(angle) 0 cos(angle)];
Z=[cos(angle) -sin(angle) 0; sin(angle) cos(angle) 0; 0 0 1];
switch axis
    case 'x'
        PR=P*transpose(X);
    case 'y'
        PR=P*transpose(Y);
    case 'z'
        PR=P*transpose(Z);
end
end  
