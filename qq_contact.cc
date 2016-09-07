//
// Created by lan on 16-9-5.
//

#include "qq_contact.h"

qq_core::QQContact::QQContact(qq_core::HttpClient &client, map<string, qq_core::Header> need) {
    this->client_ = &client;
    this->need_ = need;
    this->qqFriends_ = new QQFriend();
}
qq_core::QQContact::QQContact() {

}
qq_core::QQContact::~QQContact() {
    if(this->qqFriends_){
        delete  qqFriends_;
    }
}

bool qq_core::QQContact::GetUserFriends() {
    client_->setURL("http://s.web2.qq.com/api/get_user_friends2");
    client_->setTempHeaher(Header("Host","s.web2.qq.com"));
    client_->setTempHeaher(Header("Origin","http://s.web2.qq.com"));
    client_->setTempHeaher(Header("Referer","http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1"));

    Cookie uin = need_["uin"];
    long qq_id = atoi(uin.value.c_str());

    string r = "{\"vfwebqq\":\""+need_["vfwebqq"].value+"\",\"hash\":\""+GetHash(qq_id,need_["ptwebqq"].value)+"\"}";

    client_->setPostField(Field("r",client_->URLEncoded(r)));

    if(!client_->Execute(HttpClient::POST)){
        return false;
    }
    string response = client_->GetDataByString();
    cout << response <<endl;
    return PaserUserFriendsJson(response);
}
bool qq_core::QQContact::PaserUserFriendsJson(const string &json) {
    Json::Reader reader;
    Json::Value root;

    if(!reader.parse(json.c_str(),root)){
        //Json数据有误
        return false;
    }

    int retcode = root["retcode"].asInt();
    if(0 !=retcode){
        //请求不成功
        return false;
    }
    Json::Value result = root["result"];
    //获取所有分组的信息
    Json::Value categories = result["categories"];
    int count = categories.size();
    Json::Value item;
    for(int i = 0 ; i < count ; ++i){
        item = categories[i];
        FriendGroup friendGroup;
        friendGroup.index = item["index"].asInt();
        friendGroup.sort = item["sort"].asInt();
        friendGroup.name = item["name"].asString();
        qqFriends_->AddFriendGroup(friendGroup);
    }
    std::map<u_int64_t ,FriendInfo> friendInfos;
    //获取好友所在分组
    Json::Value friends = result["friends"];
    count = friends.size();
    for(int i =0 ; i < count ; ++i){
        item = friends[i];
        cout << item.toStyledString() <<endl;
        FriendInfo friendInfo;
        friendInfo.id = item["uin"].asUInt64();
        friendInfo.group_index = item["categories"].asInt();
        friendInfos.insert(pair<u_int64_t,FI>(friendInfo.id,friendInfo));
    }
    //获取好友的昵称,头像信息
    Json::Value infos = result["info"];
    count = infos.size();
    for(int i =0 ; i < count ; ++i){
        item = infos[i];
        cout << item.toStyledString() <<endl;
        u_int64_t  id_t = item["uin"].asUInt64();
        friendInfos[id_t].face = item["face"].asInt();
        friendInfos[id_t].nick_name = item["nick"].asString();
    }
    //获取所有好友备注信息
    Json::Value mark_names = result["marknames"];
    count = mark_names.size();
    for(int i =0 ; i < count ; ++i){
        item = mark_names[i];
        cout << item.toStyledString() <<endl;
        u_int64_t  id_t = item["uin"].asUInt64();
        friendInfos[id_t].mark_name = item["markname"].asString();
    }
    //获取所有好友vip信息
    Json::Value vips = result["vipinfo"];
    count = mark_names.size();
    for(int i =0 ; i < count ; ++i){
        item = mark_names[i];
        cout << item.toStyledString() <<endl;
        u_int64_t  id_t = item["u"].asUInt64();
        friendInfos[id_t].vip_level = item["vip_level"].asInt();
        int vipCode = item["is_vip"].asInt();
        friendInfos[id_t].is_vip = (0 == vipCode?false:true);
    }
    qqFriends_->AddAllFriendsInfo(friendInfos);
    return true;
}
const string qq_core::QQContact::GetHash(const long &uin, const string &ptwebqq) {
    int a[4] = {0};
    for (int i = 0; i < ptwebqq.length(); ++i) {
        a[i % 4] ^= ptwebqq[i];
    }
    char w[4] = {'E', 'C', 'O', 'K'};
    long d[4]={0};
    long lb = uin;
    d[0] = lb >> 24 & 255 ^ w[0];
    d[1] = lb >> 16 & 255 ^ w[1];
    d[2] = lb >> 8 & 255 ^ w[2];
    d[3] = lb & 255 ^ w[3];

    char a1[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char dl[16]={0};
    for (int i = 0; i < 8; ++i) {
        long t = (i % 2 == 0 ? a[i >> 1] : d[i >> 1]);
        dl[i * 2] = a1[(int) (t >> 4 & 15)];
        dl[i * 2 + 1] = a1[(int) (t & 15)];
    }

    return string(dl, 0, 16);
}

bool qq_core::QQContact::GetGroupList() {
    client_->setURL("http://s.web2.qq.com/api/get_group_name_list_mask2");
    client_->setTempHeaher(Header("Host","s.web2.qq.com"));
    client_->setTempHeaher(Header("Origin","http://s.web2.qq.com"));
    client_->setTempHeaher(Header("Referer","http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1"));

    Cookie uin = need_["uin"];
    long qq_id = atoi(uin.value.c_str());

    string r = "{\"vfwebqq\":\""+need_["vfwebqq"].value+"\",\"hash\":\""+GetHash(qq_id,need_["ptwebqq"].value)+"\"}";

    client_->setPostField(Field("r",client_->URLEncoded(r)));

    if(!client_->Execute(HttpClient::POST)){
        return false;
    }
    string response = client_->GetDataByString();
    cout << response <<endl;
    return PaserGroupInfoJson(response);
}

bool qq_core::QQContact::PaserGroupInfoJson(const string &json) {
    Json::Reader reader;
    Json::Value root;

    if(!reader.parse(json.c_str(),root)){
        //Json数据有误
        return false;
    }

    int retcode = root["retcode"].asInt();
    if(0 !=retcode){
        //请求不成功
        return false;
    }
    Json::Value namelist = root["result"]["gnamelist"];
    int count = namelist.size();
    Json::Value item;
    for(int i = 0 ;i < count ; ++i){
        item = namelist[i];
        GroupInfo groupInfo;
        groupInfo.id = item["gid"].asUInt64();
        groupInfo.name = item["name"].asString();
        qqFriends_->AddGroupInfo(groupInfo);
    }
    return true;
}

bool qq_core::QQContact::GetDicusList() {
    string url = "http://s.web2.qq.com/api/get_discus_list?clientid=53999199& psessionid="
                 +need_["psessionid"].value
                 +"vfwebqq="
                 +need_["vfwebqq"].value
                 +"&t=1473237774012";
    client_->setURL(url);
    client_->setTempHeaher(Header("Host","s.web2.qq.com"));
    client_->setTempHeaher(Header("Referer","http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1"));

    if(!client_->Execute(HttpClient::GET)){
        return false;
    }
    string response = client_->GetDataByString();
    cout << response <<endl;
    return PaserDiscusInfoJson(response);
}

bool qq_core::QQContact::PaserDiscusInfoJson(const string &json) {
    Json::Reader reader;
    Json::Value root;

    if(!reader.parse(json.c_str(),root)){
        //Json数据有误
        return false;
    }

    int retcode = root["retcode"].asInt();
    if(0 !=retcode){
        //请求不成功
        return false;
    }
    Json::Value namelist = root["result"]["dnamelist"];
    int count = namelist.size();
    Json::Value item;
    for(int i = 0 ;i < count ; ++i){
        item = namelist[i];
        DI di;
        di.id = item["did"].asUInt64();
        di.name = item["name"].asString();
        qqFriends_->AddDiscusnfo(di);
    }
    return true;
}

bool qq_core::QQContact::GetSelfInfo(QI &qi) {
    string url = "http://s.web2.qq.com/api/get_self_info2?t=1473237774012";
    client_->setURL(url);
    client_->setTempHeaher(Header("Host","s.web2.qq.com"));
    client_->setTempHeaher(Header("Referer","http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1"));

    if(!client_->Execute(HttpClient::GET)){
        return false;
    }
    string response = client_->GetDataByString();
    cout << response <<endl;
    return PaserQQInfo(response,qi);
}

bool qq_core::QQContact::PaserQQInfo(const string &json, qq_core::QI &qi) {
    Json::Reader reader;
    Json::Value root;

    if(!reader.parse(json.c_str(),root)){
        //Json数据有误
        return false;
    }

    int retcode = root["retcode"].asInt();
    if(0 !=retcode){
        //请求不成功
        return false;
    }
    Json::Value result = root["result"];
    qi.id = result["uin"].asUInt64();
    qi.allow = result["allow"].asInt();
    qi.year = result["year"]["birthday"].asInt();
    qi.month = result["month"]["birthday"].asInt();
    qi.day = result["day"]["birthday"].asInt();
    qi.blood = result["blood"].asInt();
    qi.city = result["city"].asString();
    qi.collage = result["collage"].asString();
    qi.constel = result["constel"].asInt();
    qi.country = result["country"].asString();
    qi.email = result["email"].asString();
    qi.face = result["face"].asInt();
    qi.gender = result["gender"].asString();
    qi.homepage = result["homepage"].asString();
    qi.mobile = result["mobile"].asString();
    qi.nick = result["nick"].asString();
    qi.occupation = result["occupation"].asString();
    qi.personal = result["personal"].asString();
    qi.phone = result["phone"].asString();
    qi.province = result["province"].asString();
    qi.shengxiao = result["shengxiao"].asInt();
    qi.stat = result["stat"].asInt();
    qi.vip_info = result["vip_info"].asInt();
    return true;
}

bool qq_core::QQContact::GetRecentList() {
    client_->setURL("http://d1.web2.qq.com/channel/get_recent_list2");
    client_->setTempHeaher(Header("Host","d1.web2.qq.com"));
    client_->setTempHeaher(Header("Origin","http://d1.web2.qq.com"));
    client_->setTempHeaher(Header("Referer","http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2"));

    Cookie uin = need_["uin"];
    long qq_id = atoi(uin.value.c_str());

    string r ="r:{\"vfwebqq\":\""+need_["vfwebqq"].value+"\",\"clientid\":53999199,\"psessionid\":\""+need_["psessionid"].value+"\"}";
    client_->setPostField(Field("r",client_->URLEncoded(r)));

    if(!client_->Execute(HttpClient::POST)){
        return false;
    }
    string response = client_->GetDataByString();
    cout << response <<endl;
    return PaserRecentListJson(response);
}

bool qq_core::QQContact::PaserRecentListJson(const string &json) {
    Json::Reader reader;
    Json::Value root;

    if(!reader.parse(json.c_str(),root)){
        //Json数据有误
        return false;
    }

    int retcode = root["retcode"].asInt();
    if(0 !=retcode){
        //请求不成功
        return false;
    }
    Json::Value result = root["result"];
    RI ri;
    ri.id = result["uin"].asUInt64();
    ri.type = result["type"].asInt();
    return true;
}

void qq_core::QQContact::set_client(qq_core::HttpClient &client) {
    this->client_ = &client;
}

void qq_core::QQContact::set_need(map<string, qq_core::Header> need) {
    this->need_ = need;
}




