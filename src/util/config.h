#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <string>
#include <utility>

using json = nlohmann::json;

class Config {
public:
  static std::string path;

  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;
  static Config &get_instance() {
    static Config config(path);
    return config;
  }

  const json &get_json() { return config_; }

  static void set_path(std::string _path) { path = _path; }
  static std::string get_path() { return path; }

  template <typename Valuetype> Valuetype get(const std::string &key) {
    return config_[key].get<Valuetype>();
  }

  std::string dump(bool pretty = true) {
    if (pretty) {
      return config_.dump(4);
    }
    return config_.dump();
  }

private:
  json config_;

  Config(std::string path) {
    if (path == "") {
      path = "../config.json";
    }

    std::ifstream f(path);
    if (!f.good()) {
      std::cerr << "path: " << path << " doesn't exist" << std::endl;
      exit(EXIT_FAILURE);
    }
    config_ = json::parse(f);
  }

  void update_config() {
    if (path == "") {
      path = "../config.json";
    }

    std::ifstream f(path);
    if (!f.good()) {
      std::cerr << "path: " << path << " doesn't exist" << std::endl;
      exit(EXIT_FAILURE);
    }
    config_ = json::parse(f);
  }
};

inline std::pair<std::string, uint16_t>
parse_config(std::string path = "../config.json") {
  std::ifstream f(path);
  json config = json::parse(f);

  std::string ip = config["Host"];
  uint16_t port = config["Port"];

  return std::make_pair(ip, port);
}
#endif /* CONFIG_H */
