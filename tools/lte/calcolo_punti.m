function [punti,zsp,lato,high] = calcolo_punti
%restituisce una matrice di coordinate a tre variabili (x,y,z), che simula
%l'andamento a serpentina in un campo quadrato, con lato scelto
%dall'utente. Supponiamo che l'angolo di apertura della fotocamera del
%drone sia 90 gradi.
high=input('Inserire altezza di volo del drone: ');
lato=input('Inserire lato del campo: ');
if mod(lato,2*high) ~= 0                 %se il lato del campo non e' multiplo del lato della foto
    while (mod(lato,2*high) ~= 0)        %non si copre perfettamente l'intero campo
        lato = lato+1;
    end
    disp('Si considera un lato di: ');
    disp(lato);
end
sup=lato^2;
disp('Superficie totale = ');
disp(sup);
area_foto=(high*2)^2;
n_foto = sup/area_foto;
A = high;

%ASCISSE
for i=1:lato/(2*high)           %creo il primo vettore di coordinate x ascendenti
    a(i)= A;
    A = A + 2*high;
end
i=1;
k=size(a,2);
for (j=1:2*(lato/(2*high)))     %coordinate x ascendenti e discendenti
    if j<=lato/(2*high)
        v(j)=a(i);
        i=i+1;
    else v(j) = a(k);
        k=k-1;
    end
end
j=1;
i=1;
while (i<=n_foto)               %ripeto il vettore per coprire tutto il campo
    x(i)=v(j);
    j=j+1;
    if j == size(v,2)
        j = 1;
        x(i+1)=v(j);
        i=i+1;
    end
    i=i+1;
end
for i=2:n_foto+1
    X(i)=x(i-1);
end

%ORDINATE
B = high;                       %creo il primo vettore di ordinate
for (i=1:lato/(2*high))
    b(i) = B;
    B = B +2*high;
end
k=1;
for(i=1:n_foto)                          %ripeto gli elementi di b per il numero
    y(i)=b(k);                           %di foto scattate per ogni fila del campo
    if (mod(i,lato/(2*high)))==0 && i~=1
        k=k+1;
    end
end
Y(1)=0;
for i=2:n_foto+1
    Y(i)=y(i-1);
end

%ALTEZZA
h(1)=0;
for i=2:n_foto+1
    h(i)=high;                  %ipotizzo costante l'altezza di volo del drone
end

%COORDINATE COMPLETE
punti = [X' Y' h'];     %unisco i tre vettori in un'unica matrice
punti(i+1,1)=X(i);
punti(i+1,2)=Y(i);
punti(i+1,3)=0;

%COORDINATE ZSP
zsp = [lato/2 lato/2 0];
return
end