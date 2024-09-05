UNAME_S != uname -s
UNAME_M != uname -m
OS = ${UNAME_S}
ARCH = ${UNAME_M}

.if ${UNAME_S} == "OpenBSD"
OS = openbsd
.elif ${UNAME_S} == "NetBSD"
OS = netbsd
.elif ${UNAME_S} == "FreeBSD"
OS = freebsd
.elif ${UNAME_S} == "Dragonfly"
OS = dragonfly
.elif ${UNAME_S} == "Linux"
OS = linux
.endif

.if ${UNAME_M} == "X86_64
ARCH = amd64
.endif

NAME != cat main.cc | grep "const char \*sofname" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"
VERSION != cat main.cc | grep "const char \*version" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"

PREFIX = /usr/local
.if ${OS} == "linux"
PREFIX = /usr
.endif

CC = c++
FILES = main.cc

.if ${OS} == "openbsd"
DEPS = pkg_add fltk jpeg png
.elif ${OS} == "netbsd"
DEPS = pkgin install fltk libjpeg-turbo png
.elif ${OS} == "freebsd"
DEPS = pkg install fltk libjpeg-turbo png
.endif

CFLAGS = -Wall -Wextra -Wno-non-c-typedef-for-linkage -I/usr/include -L/usr/lib
.if ${OS} == "freebsd" || ${OS} == "openbsd" || ${OS} == "netbsd" || ${OS} == "dragonfly
CFLAGS += -I/usr/local/include -L/usr/local/lib
.endif
.if ${OS} == "netbsd"
CFLAGS += -I/usr/X11R7/include -L/usr/X11R7/lib -I/usr/pkg/include -L/usr/pkg/lib
.elif ${OS} == "openbsd"
CFLAGS += -I/usr/X11R6/include -L/usr/X11R6/lib
.endif

LDFLAGS = -lfltk -lfltk_images -lX11
.if ${OS} == "netbsd"
LDFLAGS += -lpng16 -ljpeg
.endif

SLIB = -lc++
.if ${OS} == "openbsd"
SLIB = -lc++abi -lpthread -lm -lc \
			 -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lXdmcp -lXau \
			 -lpng -lz -ljpeg -lxcb -lXrender -lexpat -lfreetype
.elif ${OS} == "freebsd"
SLIB = -lcxxrt -lm -lgcc -lXrender -lXcursor -lXfixes -lXext -lXft -lfontconfig\
			 -lXinerama -lthr -lpng16 -lz -ljpeg -lxcb -lfreetype -lexpat -lXau -lXdmcp\
			 -lbz2 -lbrotlidec -lbrotlicommon
.elif ${OS} == "netbsd"
SLIB = -lstdc++ -lpthread -lm -lc -lXft -lxcb -lfontconfig -lfreetype\
			 -lXau -lXdmcp -lXcursor -lXrandr -lXext -lXrender -lXfixes -lXinerama -lX11\
			 -lexpat -lz -lbz2 -lgcc
.elif ${OS} == "linux"
SLIB = -lstdc++ -lgcc -lc -lXft -lXext -lXrender -lfontconfig -lXinerama\
			 -lpng16 -lz -ljpeg -lxcb -lfreetype -lexpat -lXau -lXdmcp -lbz2\
			 -lbrotlidec -lbrotlicommon
.endif

all:
	${CC} -O3 ${CFLAGS} -o ${NAME}\
		${FILES} -static ${LDFLAGS} ${SLIB}
	strip ${NAME}

depend:
	${DEPS}

debug:
	${CC} -g ${CFLAGS} -o ${NAME} ${FILES} ${LDFLAGS}

clean:
	rm -rf ${NAME}

dist:
	mkdir -p ${NAME}-${VERSION} release/src
	cp -R LICENSE.txt Makefile README.md CHANGELOG.md\
		main.c src ${NAME}-${VERSION}
	tar zcfv release/src/${NAME}-${VERSION}.tar.gz ${NAME}-${VERSION}
	rm -rf ${NAME}-${VERSION}

release:
	mkdir -p release/bin/${VERSION}/${OS}/${ARCH}
	${CC} ${CFLAGS} -o release/bin/${VERSION}/${OS}/${ARCH}/${NAME} ${FILES}\
		-static ${LDFLAGS}
	strip release/bin/${VERSION}/${OS}/${ARCH}/${NAME}

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}

.PHONY: all debug clean dist release install uninstall
