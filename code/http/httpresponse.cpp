/*
 * @Author       : mark
 * @Date         : 2020-06-27
 * @copyleft Apache 2.0
 */
#include "httpresponse.h"

using namespace std;


// DataSet* HttpResponse::dataset = new DataSet(); 


const unordered_map <string, string> HttpResponse::SUFFIX_TYPE = {
        {".html",  "text/html"},
        {".xml",   "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt",   "text/plain"},
        {".rtf",   "application/rtf"},
        {".pdf",   "application/pdf"},
        {".word",  "application/nsword"},
        {".png",   "image/png"},
        {".gif",   "image/gif"},
        {".jpg",   "image/jpeg"},
        {".jpeg",  "image/jpeg"},
        {".au",    "audio/basic"},
        {".mpeg",  "video/mpeg"},
        {".mpg",   "video/mpeg"},
        {".avi",   "video/x-msvideo"},
        {".gz",    "application/x-gzip"},
        {".tar",   "application/x-tar"},
        {".css",   "text/css "},
        {".js",    "text/javascript "},
        {".json",  "application/json"},

};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
        {200, "OK"},
        {400, "Bad Request"},
        {403, "Forbidden"},
        {404, "Not Found"},
};

const unordered_map<int, string> HttpResponse::CODE_PATH = {
        {400, "/400.html"},
        {403, "/403.html"},
        {404, "/404.html"},
};

HttpResponse::HttpResponse() {
    code_ = -1;
    path_ = srcDir_ = "";
    isKeepAlive_ = false;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
    dataset = new DataSet(); 

};

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const string &srcDir, string &path, bool isKeepAlive, int code) {
    assert(srcDir != "");
    if (mmFile_) { UnmapFile(); }
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
}


void HttpResponse::FindDataset() {
    if (path_.find("?") < path_.size()) {
        cout<< path_ <<endl;
        Json::StyledWriter sw;
        string source = path_.substr(0, path_.find("?"));
        string api = path_.substr(path_.find("?") + 1);
        string param = api.substr(api.find("=") + 1);
        api = api.substr(0, api.find("="));
        struct data_line res;
        if (source == "/api/more/FindById") {
            if(!dataset->FindById(param, res)){
                dataset->FindByLine(std::to_string(0), res);
            }
        } else if (source == "/api/more/FindByLine") {
            if(!dataset->FindByLine(param, res)){
                dataset->FindByLine(std::to_string(0), res);
            }
        }
        // 创建图片
        string img_path = "/home/wanghk/code/MORE/ours/images/" + res.img_id;
        cv::Mat src = cv::imread(img_path);
        int mw = src.cols, mh = src.rows;
        int w = mw * std::get<2>(res.obj.position), h = mh * std::get<3>(res.obj.position);
        int x = std::get<0>(res.obj.position) * mw - w / 2.0, y = std::get<1>(res.obj.position) * mh - h / 2.0;
        cv::Rect rect(x, y, w, h);// 左上坐标（x,y）和矩形的长 (x) 宽(y)
        cv::rectangle(src, rect, cv::Scalar(0, 0, 255), 3,  0);
        string out_path = "api/more/imgs/" + std::to_string(res.line_id) + ".png";
        cv::imwrite(srcDir_ + out_path, src);
        //根节点
        Json::Value root;
        root["img_path"] = Json::Value(out_path);
        root["title"] = Json::Value(res.text);
        root["trans"] = Json::Value(res.trans);
        root["entity"] = Json::Value(res.ent.ent_name);
        root["object"] = Json::Value(res.obj.obj_name);
        root["img_id"] = Json::Value(res.img_id);
        root["line_id"] = Json::Value(res.line_id);
        root["relation"] = Json::Value(res.relation);

        path_ = "api/more/" + std::to_string(res.line_id) + ".json";
        if (std::fstream(srcDir_ + path_)) {
            return;
        }
        //输出到文件
        ofstream os(srcDir_ + path_, std::ios::out | std::ios::app);
        if (!os.is_open())
            LOG_DEBUG("Error：can not find or create the json file.")
        else {
            os << sw.write(root);
            os.close();
        }
        return;
    }
}

void HttpResponse::MakeResponse(Buffer &buff) {
    /* 判断请求的资源文件 */
    cout << srcDir_ << " " << path_ << endl;
    FindDataset();
    if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;
    } else if (!(mmFileStat_.st_mode & S_IROTH)) {
        code_ = 403;
    } else if (code_ == -1) {
        code_ = 200;
    }
    ErrorHtml_();
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

char *HttpResponse::File() {
    return mmFile_;
}

size_t HttpResponse::FileLen() const {
    return mmFileStat_.st_size;
}

void HttpResponse::ErrorHtml_() {
    if (CODE_PATH.count(code_) == 1) {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

void HttpResponse::AddStateLine_(Buffer &buff) {
    string status;
    if (CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer &buff) {
    buff.Append("Access-Control-Allow-Origin: http://localhost:8080 \r\n");
    buff.Append("Connection: ");
    if (isKeepAlive_) {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType_() + "\r\n");
}

void HttpResponse::AddContent_(Buffer &buff) {
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if (srcFd < 0) {
        ErrorContent(buff, "File NotFound!");
        return;
    }

    /* 将文件映射到内存提高文件的访问速度 
        MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    LOG_DEBUG("file path %s", (srcDir_ + path_).data());
    int *mmRet = (int *) mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (*mmRet == -1) {
        ErrorContent(buff, "File NotFound!");
        return;
    }
    mmFile_ = (char *) mmRet;
    close(srcFd);
    buff.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

void HttpResponse::UnmapFile() {
    if (mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}

string HttpResponse::GetFileType_() {
    /* 判断文件类型 */
    string::size_type idx = path_.find_last_of('.');
    if (idx == string::npos) {
        return "text/plain";
    }
    string suffix = path_.substr(idx);
    if (SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::ErrorContent(Buffer &buff, string message) {
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}
