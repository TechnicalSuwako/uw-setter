#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <sys/stat.h>
#include <dirent.h>

std::string sfp;
Fl_Button *choose_button;
Fl_Button *set_button;

const char *sofname = "uw-setter";
const char *version = "0.1.0";

void set_wallpaper(const std::string &filepath) {
  std::string command = "uw -t \"" + filepath + "\"";
  system(command.c_str());
  set_button->deactivate();
}

class ClickableBox : public Fl_Box {
public:
  ClickableBox(int X, int Y, int W, int H, const char *L = 0)
    : Fl_Box(X, Y, W, H, L), image_path("") {}

  int handle(int event) override {
    switch (event) {
      case FL_PUSH:
        set_wallpaper(get_image_path());
        alterXinitrc(get_image_path());
        return 1;
      default:
        return Fl_Box::handle(event);
    }
  }

  void set_image_path(const std::string &path) {
    image_path = path;
  }

  std::string get_image_path() const {
    return image_path;
  }

private:
  std::string image_path;

  void alterXinitrc(const std::string &npath) {
    const std::string xinitrc = std::string(getenv("HOME")) + "/.xinitrc";
    std::ifstream in(xinitrc);
    std::vector<std::string> lines;
    std::string line;
    bool founduw = false;
    bool addeduw = false;

    while (std::getline(in, line)) {
      if (line.rfind("uw -", 0) == 0) {
        line = "uw -t " + npath + " &";
        founduw = true;
      }
      lines.push_back(line);

      if (line.rfind("exec ", 0) == 0 && !founduw && !addeduw) {
        lines.insert(lines.end() - 1, "uw -t " + npath + " &");
        addeduw = true;
      }
    }
    in.close();

    if (!founduw && !addeduw) {
      lines.push_back("uw -t " + npath + " &");
    }

    std::ofstream out(xinitrc);
    for (const auto &eline : lines) {
      out << eline << std::endl;
    }
    out.close();
  }
};

std::vector<std::string> list_images(const std::string &directory) {
  std::vector<std::string> images;
  DIR *dir = opendir(directory.c_str());
  if (!dir) {
    std::cerr << "ディレクトリを開くに失敗： " << directory << std::endl;
    return images;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string filename = entry->d_name;
    if (filename.size() <= 4) continue;

    std::string ext = filename.substr(filename.size() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".png" || ext == ".jpg") {
      std::string tmp = directory;
      tmp.append("/").append(filename);
      images.push_back(tmp);
    }
  }

  closedir(dir);
  return images;
}

void thumbnail_cb(Fl_Widget *widget, void *userdata) {
  (void)widget;
  const char *filepath = static_cast<const char *>(userdata);
  if (filepath) {
    set_wallpaper(filepath);
  }
}

bool isExist(const std::string &path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0) return false;
  else if (info.st_mode & S_IFDIR) return true;
  return false;
}

void set_wallpaper_cb(Fl_Widget *widget, void *userdata) {
  (void)widget;
  (void)userdata;
  if (!sfp.empty()) {
    set_wallpaper(sfp);
  }
}

void choose_image_cb(Fl_Widget *widget, void *userdata) {
  (void)widget;
  const char *homedir = getenv("HOME");
  std::string defpath;

  if (homedir) {
    std::string userdir = std::string(homedir) + "/.local/share/wallpapers";
    std::string localusr = "/usr/local/share/wallpapers";
    std::string sysusr = "/usr/share/wallpapers";
    if (isExist(userdir)) {
      defpath = userdir;
    } else if (isExist(localusr)) {
      defpath = localusr;
    } else {
      defpath = sysusr;
    }
  }

  Fl_File_Chooser chooser(
    defpath.c_str(),
    "*.{png,jpg}",
    Fl_File_Chooser::SINGLE,
    "画像を語選択下さい・・・"
  );
  chooser.show();

  while (chooser.shown()) {
    Fl::wait();
  }

  if (chooser.value() != nullptr) {
    sfp = chooser.value();

    Fl_Button* set_button = static_cast<Fl_Button*>(userdata);
    set_button->activate();
  }
}

void set_dark_theme() {
  Fl::background(35, 32, 35);
  Fl::background2(68, 59, 68);
  Fl::foreground(252, 252, 252);
}

int main(int argc, char **argv) {
  Fl_Window *window = new Fl_Window(800, 800, "uw-setter");

  set_dark_theme();

  // ボタン
  choose_button = new Fl_Button(10, 740, 200, 40, "画像を選択");
  set_button = new Fl_Button(230, 740, 200, 40, "背景の設置");
  set_button->deactivate();

  // スクロールウィンドウ
  Fl_Scroll *scroll = new Fl_Scroll(10, 10, 780, 720);
  scroll->type(Fl_Scroll::VERTICAL);

  std::vector<std::string> imgfiles;
  std::vector<Fl_Image *> thumbs;

  // ボタンで選択
  choose_button->callback(choose_image_cb, (void*)set_button);
  set_button->callback(set_wallpaper_cb);

  // サムネール
  const char *home = getenv("HOME");
  std::string homedir = "";
  if (home) homedir = std::string(home) + "/.local/share/wallpapers";
  std::string localdir = "/usr/local/share/wallpapers";
  std::string sysdir = "/usr/share/wallpapers";

  if (isExist(homedir)) {
    std::vector<std::string> img = list_images(homedir);
    imgfiles.insert(imgfiles.end(), img.begin(), img.end());
  }
  if (isExist(localdir)) {
    std::vector<std::string> img = list_images(localdir);
    imgfiles.insert(imgfiles.end(), img.begin(), img.end());
  }
  if (isExist(sysdir)) {
    std::vector<std::string> img = list_images(sysdir);
    imgfiles.insert(imgfiles.end(), img.begin(), img.end());
  }

  int x = 10, y = 10;
  for (const auto &filepath : imgfiles) {
    Fl_Image* img = nullptr;
    std::string ext = filepath.substr(filepath.size() - 4);

    if (ext == ".png") {
      img = new Fl_PNG_Image(filepath.c_str());
    } else if (ext == ".jpg") {
      img = new Fl_JPEG_Image(filepath.c_str());
    }

    if (!img || img->w() <= 0 || img->h() <= 0) continue;

    Fl_Image *thumb = img->copy(100, 100);
    thumbs.push_back(thumb);
    delete img;

    /* Fl_Box *box = new Fl_Box(x, y, 100, 100); */
    ClickableBox *box = new ClickableBox(x, y, 100, 100);
    box->image(thumb);
    box->box(FL_FLAT_BOX);
    box->set_image_path(filepath.c_str());

    x += 100;
    if (x + 100 > scroll->w()) {
      x = 10;
      y += 110;
    }
  }

  scroll->end();
  window->end();
  window->show(argc, argv);

  int ret = Fl::run();

  for (auto img : thumbs) {
    delete img;
  }

  return ret;
}
