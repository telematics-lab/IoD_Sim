# Installazione guidata di IoD_Sim

1. Assicurarsi di possedere Windows 10 versione 2004 o successive.
    Per informazioni sulla versione avviare Esegui premento tasto `Win + R`, scrivere `winver` e premere Invio.
2. Installare Windows Subsystem for Linux versione 2.
    Maggiori informazioni: https://docs.microsoft.com/en-us/windows/wsl/install-win10
3. Installare le dipendenze. Per Ubuntu le dipendenze sono le seguenti:
    ```
    apt install g++ python3 python3-dev pkg-config sqlite3 python3-setuptools git gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3 valgrind uncrustify gsl-bin libgsl-dev libgsl23 libgslcblas0 tcpdump libxml2 libxml2-dev rapidjson-dev python-is-python3
    ```
4. Scaricare o effettuare il clone del repository di IoD_Sim sulla propria owkring directory
5. Entrare nella directory di IoD_Sim
6. Scaricare l'ultima versione supportata di ns-3:
    `git clone --branch=ns-3.29 https://gitlab.com/nsnam/ns-3-dev ns3`
7. ```
    cd ns3/src
    ln -s ../../drone drone
    ln -s ../../report report
    cd ..
    ```
8. `./waf configure --enable-examples`
9. Determina il numero di core/thread: `grep -c processor /proc/cpuinfo`
10. `./waf build -j<CORES>`
    dove al posto di `<CORES>` ci deve essere il numero tornato dal comando al punto 9.

# Sviluppo con IoD_Sim
1. Installa Visual Studio Code
2. Installa l'estensione `Remote - WSL`. Impara ad usarlo.
3. Installa l'estensione `C/C++` di Microsoft.

4. Enjoy!

