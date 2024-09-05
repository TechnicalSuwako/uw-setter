# uw-setter
GUIで使ってuwをコントロール出来る

## インストールする方法 | Installation
### BSD
```sh
doas make depends
make
doas make install
```

### Void Linux
```sh
sudo xbps-install fltk fltk-devel libjpeg-turbo libjpeg-turbo-devel libpng libpng-devel
bmake
sudo bmake install
```

### Alpine Linux
```sh
sudo apk add fltk fltk-dev libjpeg-turbo libjpeg-turbo-dev libpng libpng-dev
bmake
sudo bmake install
```
