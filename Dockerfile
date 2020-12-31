FROM debian:stable-slim
MAINTAINER rhessing

ENV DOWNLOAD_URL="https://github.com/rhessing/ssdb/archive/master.zip"

# Basics
RUN apt-get -y update && \
  apt-get install --force-yes -y python unzip wget make autoconf g++ && \
  wget --no-check-certificate ${DOWNLOAD_URL} && \
  unzip master && \
  cd ssdb-master && \
  make && make install && \
  mkdir -p /var/lib/ssdb && \
  ln -fsn /usr/local/ssdb/ssdb-cli /bin/ssdb-cli && \
  ln -fsn /usr/local/ssdb/ssdb-bench /bin/ssdb-bench && \
  ln -fsn /usr/local/ssdb/ssdb-dump /bin/ssdb-dump && \
  ln -fsn /usr/local/ssdb/ssdb-repair /bin/ssdb-repair && \
  ln -fsn /usr/local/ssdb/leveldb-import /bin/leveldb-import && \
  cd

COPY docker-entrypoint.sh /usr/local/bin/
RUN chmod 755 /usr/local/bin/docker-entrypoint.sh

# clean up
RUN rm -rf /ssdb-master /master.zip && \
  rm -f /usr/local/ssdb/Makefile && \
  rm -f docker-entrypoint.sh && \
  rm -f Dockerfile && \
  rm -f README.md && \
  apt-get purge -y \
    autotools-dev binutils binutils-common binutils-x86-64-linux-gnu \
    bzip2 ca-certificates cpp cpp-8 file g++-8 gcc gcc-8 libasan5 libatomic1 \
    libbinutils libc-dev-bin libc6-dev libcc1-0 libexpat1 libgcc-8-dev \
    libgdbm-compat4 libgdbm6 libgomp1 libisl19 libitm1 liblsan0 libmagic-mgc \
    libmagic1 libmpc3 libmpfr6 libmpx2 libpcre2-8-0 libperl5.28 libpsl5 \
    libquadmath0 libreadline7 libsigsegv2 libsqlite3-0 libssl1.1 libstdc++-8-dev \
    libtsan0 libubsan1 linux-libc-dev m4 manpages manpages-dev mime-support \
    netbase openssl perl perl-modules-5.28 publicsuffix readline-common xz-utils \
    unzip wget make autoconf g++ && \
  apt-get autoremove -y && \
  apt-get clean -y


ENV TZ Europe/Amsterdam
EXPOSE 8888
VOLUME /var/lib/ssdb
CMD ["/usr/local/bin/docker-entrypoint.sh"]
