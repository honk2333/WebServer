//
// Created by wanghk on 23-5-11.
//

#ifndef WEBSERVER_DATA_H
#define WEBSERVER_DATA_H


#include <bits/stdc++.h>
#include "../log/log.h"
#include <jsoncpp/json/json.h>

struct entity {
    std::string ent_name;
    std::tuple<int, int> position;

    entity() = default;

    entity(std::string ent, std::tuple<int, int> pos) : ent_name(ent), position(pos) {};

};

struct object {
    std::string obj_name;
    std::tuple<double, double, double, double> position;

    object() = default;

    object(std::string obj, std::tuple<double, double, double, double> pos) : obj_name(obj), position(pos) {};

};

struct data_line {
    struct entity ent;
    struct object obj;
    int line_id;
    std::string text;
    std::string trans;
    std::string img_id;
    std::string relation;
    data_line() = default;
    data_line(struct entity ent, struct object obj, int line_id, std::string img_id, std::string text, std::string trans, std::string rel)
            : ent(ent),
              obj(obj),
              line_id(line_id),
              img_id(img_id),
              relation(rel),
              text(text),
              trans(trans) {};
};


class DataSet {
public:
    DataSet();

    ~DataSet();

    void init(std::string);

    int FindById(std::string, struct data_line&) const;

    int FindByLine(std::string, struct data_line& ) const;

private:
    std::string data_path;
    int length;
    std::unordered_map<std::string, struct data_line> img_id_map;
    std::vector<struct data_line> data_vector;

};


#endif //WEBSERVER_DATA_H
