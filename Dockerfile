FROM ubuntu:latest as intermediate

RUN apt-get update -y
RUN apt-get install -y autoconf automake build-essential gcc g++ python git bison libbison-dev
RUN apt-get install -y libbsd-dev libevent-dev libpq-dev libz-dev libssl-dev libpcre3-dev
RUN apt-get install -y libmysqlclient-dev libsqlite3-dev
RUN apt-get install -y libgtest-dev googletest
RUN apt-get install -y curl

#RUN git clone https://github.com/fluffos/fluffos.git /tmp/fluffos
RUN touch /tmp/somethingfake4.txt
RUN git clone https://github.com/jpeckham/fluffos.git /tmp/fluffos
# release 3
# RUN curl https://codeload.github.com/fluffos/fluffos/tar.gz/fluffos-3.0.20170907 --output /tmp/fluffos.tar.gz --silent \
# && cd /tmp \
# && cd /tmp \
# && tar -xvf fluffos.tar.gz \
# && mv fluffos-fluffos-3.0.20170907 fluffos

COPY local_options.fluffos /tmp/fluffos/src/local_options

RUN cd /tmp/fluffos \
&& bash ./build.FluffOS; exit 0
RUN cd /tmp/fluffos \
&& make \
&& make install

# release 3
# RUN cd /tmp/fluffos/src \
# && bash ./build.FluffOS; exit 0
# RUN cd /tmp/fluffos/src \
# && make \
# && make install
# doing stages now
# RUN cp /tmp/fluffos/bin/driver /usr/bin/driver

# CMD /home/parnell/mud/mud.sh

FROM ubuntu:latest

RUN apt-get update -y
RUN apt-get install -y libbsd-dev libevent-dev libpq-dev libz-dev libssl-dev libpcre3-dev

COPY --from=intermediate /tmp/fluffos/bin/driver /usr/bin/driver
RUN mkdir /home/parnell/mud/mudlib/log \
    && mkdir /home/parnell/mud/mudlib/log/errors

CMD /home/parnell/mud/mud.sh