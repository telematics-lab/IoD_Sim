FROM fedora:latest

WORKDIR /drone

####
# Machine Setup
####
RUN dnf update -y       \
    && dnf install -y   \
    gcc                 \
    gcc-c++             \
    cmake               \
    clang-devel         \
    llvm-devel          \
    llvm-static         \
    gdb                 \
    valgrind            \
    python3-devel       \
    git                 \
    gsl-devel           \
    qt5-devel           \
    gtk3-devel          \
    sqlite-devel        \
    libxml2-devel       \
    openmpi-devel       \
    boost-devel         \
    rapidjson-devel     \
    goocanvas2-devel    \
    gobject-introspection-devel \
    pygobject3-devel    \
    python3-gobject     \
    python3-pygraphviz  \
    doxygen             \
    graphviz-devel      \
    ImageMagick         \
    pkgconf-pkg-config  \
    tcpdump             \
    castxml             \
    python3-ipython     \
    uncrustify          \
    patch               \
    autoconf            \
    redhat-rpm-config   \
    dia                 \
    texlive             \
    texlive-latex       \
    texlive-fncychap    \
    texlive-capt-of     \
    texlive-tabulary    \
    texlive-eqparbox    \
    texlive-epstopdf    \
    texlive-titlesec    \
    texlive-framed      \
    texlive-dvipng      \
    texlive-threeparttable \
    texlive-wrapfig     \
    texlive-multirow    \
    && pip3 install -U pip \
    && pip3 install -U  \
    sphinx              \
    pygccxml            \
    sphinx_rtd_theme    \
    cxxfilt		\
    && mkdir /opt/third_party \
    && cd /opt/third_party \
    && git clone https://github.com/gjcarneiro/pybindgen.git \
    && cd pybindgen     \
    && python3 setup.py install
