FROM debian:stable

ENV RUNNER_USER=runner

ENV BOOST_VER_MAJOR=1
ENV BOOST_VER_MINOR=82
ENV BOOST_VER_PATCH=0

ENV CURSES_VER_MAJOR=6
ENV CURSES_VER_MINOR=4

ENV LIBPCAP_VER_MAJOR=1
ENV LIBPCAP_VER_MINOR=10
ENV LIBPCAP_VER_PATCH=4

ENV LIBNL_VER_MAJOR=3
ENV LIBNL_VER_MINOR=7
ENV LIBNL_VER_PATCH=0

ENV GOOGLETEST_VER_MAJOR=1
ENV GOOGLETEST_VER_MINOR=13
ENV GOOGLETEST_VER_PATCH=0

RUN                                                \
  useradd -m ${RUNNER_USER}                     && \
  apt-get update                                && \
  apt-get install -y  --no-install-recommends      \
    build-essential                                \
    wget                                           \
    ca-certificates                                \
    flex                                           \
    byacc                                          \
    bison                                          \
    cmake                                          \
    git                                            \
    openssh-client                                \
    unzip                                          \
    graphviz                                       \
    pandoc                                         \
    texlive-latex-base                             \
    texlive-fonts-extra                            \
    texlive-latex-recommended                      \
    texlive-latex-extra                            \
    doxygen                                     && \
  apt-get clean                                 && \
  rm -rf /var/lib/apt/lists/*                   && \
  cd /home/${RUNNER_USER}                       && \
                                                   \
  wget https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VER_MAJOR}.${BOOST_VER_MINOR}.${BOOST_VER_PATCH}/source/boost_${BOOST_VER_MAJOR}_${BOOST_VER_MINOR}_${BOOST_VER_PATCH}.tar.bz2 && \
  wget https://ftp.gnu.org/pub/gnu/ncurses/ncurses-${CURSES_VER_MAJOR}.${CURSES_VER_MINOR}.tar.gz && \
  wget https://www.tcpdump.org/release/libpcap-${LIBPCAP_VER_MAJOR}.${LIBPCAP_VER_MINOR}.${LIBPCAP_VER_PATCH}.tar.gz && \
  wget https://github.com/thom311/libnl/releases/download/libnl${LIBNL_VER_MAJOR}_${LIBNL_VER_MINOR}_${LIBNL_VER_PATCH}/libnl-${LIBNL_VER_MAJOR}.${LIBNL_VER_MINOR}.${LIBNL_VER_PATCH}.tar.gz && \
  wget https://github.com/google/googletest/archive/refs/tags/v${GOOGLETEST_VER_MAJOR}.${GOOGLETEST_VER_MINOR}.${GOOGLETEST_VER_PATCH}.tar.gz && \
                                                                                                                   \
  for f in *.tar.*; do tar -xvf "$f"; done                                                                      && \
  rm *.tar.*                                                                                                    && \
                                                                                                                   \
  mv *boost* boost                                                                                              && \
  cd boost                                                                                                      && \
  ./bootstrap.sh                                                                                                && \
  ./b2 link=static variant=release threading=multi runtime-link=static --with-system --with-program_options     && \
  cd ..                                                                                                         && \
                                                                                                                   \
  mv *libpcap* libpcap                                                                                          && \
  cd libpcap                                                                                                    && \
  ./configure --enable-ipv6 --disable-usb --disable-dbus --without-libnl --disable-universal                    && \
  make -j`nproc`                                                                                                && \
  cd ..                                                                                                         && \
                                                                                                                   \
  mv *ncurses* ncurses                                                                                          && \
  cd ncurses                                                                                                    && \
  ./configure --with-terminfo-dirs="/etc/terminfo:/lib/terminfo:/usr/share/terminfo:/usr/lib/terminfo"             \
  --without-debug --enable-widec                                                                                && \
  make -j`nproc`                                                                                                && \
  cd ..                                                                                                         && \
                                                                                                                   \
  mv *libnl* libnl                                                                                              && \
  cd libnl                                                                                                      && \
  ./configure                                                                                                   && \
  make -j`nproc`                                                                                                && \
  cd ..                                                                                                         && \ 
													                                                                                         \
  mv *googletest* googletest                                                                                    && \
  cd googletest                                                                                                 && \
  cmake -S . -DCMAKE_CXX_STANDARD=20 -Dgtest_build_samples=OFF -Dgtest_build_tests=OFF -Dgmock_build_tests=OFF  && \
  make -j`nproc`                                                                                                && \
  cp -r googlemock/include .                                                                                    && \
  cp -r googletest/include .                                                                                    && \
  cd .. 
