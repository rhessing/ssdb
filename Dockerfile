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
  ln -fsn /usr/local/ssdb/leveldb-import /bin/leveldb-import

COPY docker-entrypoint.sh /usr/local/bin/
RUN chmod 755 /usr/local/bin/docker-entrypoint.sh

# clean up
RUN cd && rm -rf /ssdb-master /master.zip && \
  rm -f /usr/local/ssdb/Makefile && \
  apt-get purge -y unzip wget make autoconf g++ && \
  apt-get autoremove -y && \
  apt-get clean -y


ENV TZ Europe/Amsterdam
EXPOSE 8888
VOLUME /var/lib/ssdb
CMD ["/usr/local/bin/docker-entrypoint.sh"]
