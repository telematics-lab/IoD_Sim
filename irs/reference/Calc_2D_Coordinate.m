function P = Calc_2D_Coordinate(m, n, M_columns, N_rows, x_Side, y_Side)
x=x_Side*(m-(M_columns+1)/2);    %Coordinate dell'elemento (m,n) ipotizzando che la RIS sia centrata 
y=y_Side*(n-(N_rows+1)/2);       %nell'origine degli assi x/y e giacia sul piano formato da quest'ultimi
z=0;
P=[x y z];
end  
