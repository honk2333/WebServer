//
// Created by wanghk on 23-5-11.
//

#include "data.h"

using namespace std;


DataSet::DataSet() {
    this->length = 0;
    data_path = "";
    img_id_map.clear();
    data_vector.clear();
    init("/home/wanghk/code/MORE/ours/json/dataset_trans.json");
    cout << this->length << endl;
}

DataSet::~DataSet() {
    img_id_map.clear();
    data_vector.clear();
}

// DataSet::DataSet(string s){
//     this->length = 0;
//     data_path = "";
//     img_id_map.clear();
//     data_vector.clear();
//     init(s);
//     cout << this->length << endl;
// }


void DataSet::init(std::string data_path) {
    this->data_path = data_path;
    Json::Reader reader;
    Json::Value values;
    ifstream myfile(this->data_path, ios::binary);
    if (myfile.is_open()) {
        if (reader.parse(myfile, values)) {
            for (auto value: values) {
                string img_id = value["img_id"].asString();
                string text = value["text"].asString();
                string trans = value["trans"].asString();
                string ent_name = "", obj_name = value["t"]["name"].asString();
                for (auto i: value["h"]["name"]) {
                    ent_name += i.asString() + " ";
                }
                struct entity ent(ent_name,
                                  std::make_tuple(value["h"]["pos"][0].asInt(), value["h"]["pos"][1].asInt()));
                struct object obj(obj_name,
                                  std::make_tuple(value["t"]["pos"][0].asDouble(),
                                                  value["t"]["pos"][1].asDouble(),
                                                  value["t"]["pos"][2].asDouble(),
                                                  value["t"]["pos"][3].asDouble()));
                string relation = value["relation"].asString();
                struct data_line adata(ent, obj, this->length, img_id, text, trans, relation);
                this->length += 1;
                this->data_vector.push_back(adata);
                this->img_id_map.emplace(img_id, adata);
            }
        }

        myfile.close();
    } else
        LOG_DEBUG("Unable to open file %s", data_path);
    return;
}

int DataSet::FindById(string id, struct data_line& res) const {
    if (this->img_id_map.find(id) == this->img_id_map.end()) {
        LOG_DEBUG("The image id can't be find");
        return 0;
    }
    auto it = this->img_id_map.find(id);
    res = (*it).second;
    return 1;
}

int DataSet::FindByLine(string id, struct data_line& res) const {
    int line_id = stoi(id);
    cout<< line_id <<endl;
    if (line_id >= this->length) {
        LOG_DEBUG("The line id can't be find %d", line_id);
        return 0;
    }
    res = this->data_vector[line_id];
    return 1;
}