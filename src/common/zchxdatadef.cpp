#include "zchxdatadef.h"

using namespace zchxCommon;

zchxLatlon::zchxLatlon(const QJsonArray &array) : zchxData(array)
{
    lat = array[1].toDouble();
    lon = array[0].toDouble();
}

QJsonValue zchxLatlon::toJson() const
{
    QJsonArray array;
    array.append(lon);
    array.append(lat);
    return array;
}

zchxFilterArea::zchxFilterArea(const QJsonObject& obj) : zchxData(obj)
{
    id = obj.value("id").toInt();
    type = obj.value("type").toInt();
    area = zchxArea(obj.value("area").toArray());
    time = obj.value("time").toVariant().toLongLong();
    name = obj.value("name").toString();
}
QJsonValue zchxFilterArea::toJson() const
{
    QJsonObject obj;
    obj.insert("id", id);
    obj.insert("type", type);
    obj.insert("area", area.toJson());
    obj.insert("time", time);
    obj.insert("name", name);
    return obj;
}

zchxPublishSetting::zchxPublishSetting(int _id, const QString& topic_p, int port_p, const QString& desc_p) :zchxData()
{
    id = _id;
    topic = topic_p; port = port_p; desc = desc_p;
    status = false;
}
zchxPublishSetting::zchxPublishSetting(const QJsonObject& obj) : zchxData(obj)
{
    topic = obj.value("topic").toString();
    port = obj.value("port").toInt();
    desc = obj.value("desciption").toString();
    id = obj.value("id").toInt();
    status = obj.value("status").toBool();
}

QJsonValue zchxPublishSetting::toJson() const
{
    QJsonObject obj;
    obj.insert("topic", topic);
    obj.insert("port", port);
    obj.insert("desciption", desc);
    obj.insert("id", id);
    obj.insert("status", status);
    return obj;
}

zchxSocket::zchxSocket(const QString& s, int i, const QString& d) : zchxData()
{
    ip = s; port = i; desc = d; status = false;
}

zchxSocket::zchxSocket(const QJsonObject& obj):zchxData(obj)
{
    ip = obj.value("ip").toString();
    port = obj.value("port").toInt();
    desc = obj.value("desciption").toString();
    status = obj.value("status").toBool();
}
QJsonValue zchxSocket::toJson() const
{
    QJsonObject obj;
    obj.insert("ip", ip);
    obj.insert("port", port);
    obj.insert("desciption", desc);
    obj.insert("status", status);
    return obj;
}

zchxRadarChannel::zchxRadarChannel() : zchxData()
{
    video.desc = QString::fromUtf8("回波UDP设定");
    report.desc = QString::fromUtf8("状态UDP设定");
    heart.desc = QString::fromUtf8("心跳UDP设定");
    id = 0; scanline = 4096; cell = 512;
    video.ip = "236.6.7.8";
    video.port = 6678;
    video.status = false;

    report.ip = "236.6.7.9";
    report.port = 6679;
    report.status = false;

    heart.ip = "236.6.7.10";
    heart.port = 6680;
    heart.status = false;

}

zchxRadarChannel::zchxRadarChannel(const QJsonObject& obj) : zchxData(obj)
    , video(obj.value("video").toObject())
    , heart(obj.value("heart").toObject())
    , report(obj.value("report").toObject())
{
    id = obj.value("id").toInt();
    scanline = obj.value("scanline").toInt();
    cell = obj.value("cell").toInt();
}

QJsonValue zchxRadarChannel::toJson() const
{
    QJsonObject obj;
    obj.insert("id", id);
    obj.insert("video",video.toJson());
    obj.insert("report", report.toJson());
    obj.insert("heart", heart.toJson());
    obj.insert("scanline", scanline);
    obj.insert("cell", cell);
    return obj;
}

zchxRangeCheck::zchxRangeCheck(const QJsonObject& obj)
{
    id = obj.value("id").toInt();
    min = obj.value("min").toInt();
    max = obj.value("max").toInt();
    used = obj.value("used").toBool();
}
QJsonValue zchxRangeCheck::toJson() const
{
    QJsonObject obj;
    obj.insert("id", id);
    obj.insert("min", min);
    obj.insert("max", max);
    obj.insert("used", used);
    return obj;
}

zchxVideoParse::zchxVideoParse() : zchxData()
{
    lenth.id = 1;
    lenth.min = 1;
    lenth.max = 300;
    area.id = 2;
    area.min = 1;
    area.max = 90000;
    amp.id = 3;
    amp.min = 100;
    amp.max = 255;
    max_history_num = 20;
    clear_target_time = 60;
    prediction_width = 40;
    video_overlap_cnt = 1;
    max_target_speed = 70;
    scan_time = 1.5;
    radius_coeff = 2.0;
    use_video_radius = true;
    direction_invert_hold = 60;
    adjust_cog = false;
    confirm_target_cnt = 5;
    check_target_gap = false;
    output_point = true;
    output_target_min_speed = 0.0;
    prediction_enabled = false;
    manual_radius = 1000.0;
    head = 0;
    use_original_video_img = false;
    merge_video = false;
    video_color_list.append(QVariant("#ff0000"));
    video_color_list.append(QVariant("#00ff00"));
    video_color_list.append(QVariant("#0000ff"));
}

zchxVideoParse::zchxVideoParse(const QJsonObject& obj) : zchxData(obj)
{
    lenth = zchxRangeCheck(obj.value("lenth").toObject());
    area = zchxRangeCheck(obj.value("area").toObject());
    amp = zchxRangeCheck(obj.value("amp").toObject());
    max_history_num = obj.value("max_history_num").toInt();
    clear_target_time = obj.value("clear_target_time").toInt();
    prediction_width = obj.value("prediction_width").toInt();
    video_overlap_cnt = obj.value("video_overlap_cnt").toInt();
    max_target_speed = obj.value("max_target_speed").toInt();
    scan_time = obj.value("scan_time").toDouble();
    radius_coeff = obj.value("radius_coeff").toDouble();
    use_video_radius = obj.value("use_video_radius").toBool();
    direction_invert_hold = obj.value("direction_invert_hold").toDouble();
    adjust_cog = obj.value("adjust_cog").toBool();
    confirm_target_cnt = obj.value("confirm_target_cnt").toInt();
    check_target_gap = obj.value("check_target_gap").toBool();
    output_point = obj.value("output_point").toBool();
    output_target_min_speed = obj.value("output_target_min_speed").toDouble();
    prediction_enabled = obj.value("prediction_enabled").toBool();
    manual_radius = obj.value("manual_radius").toDouble();
    head = obj.value("head").toInt();
    use_original_video_img = obj.value("use_original_video_img").toBool();
    merge_video = obj.value("merge_video").toBool();
    video_color_list = obj.value("video_color_list").toArray().toVariantList();
    while (video_color_list.size() < video_overlap_cnt)
    {
        video_color_list.append(QVariant("#ff0000"));
    }

}

bool zchxVideoParse::operator ==(const zchxVideoParse& other) const
{
    return (this->adjust_cog == other.adjust_cog
            && this->amp == other.amp
            && this->area == other.area
            && this->check_target_gap == other.check_target_gap
            && this->clear_target_time == other.clear_target_time
            && this->confirm_target_cnt == other.confirm_target_cnt
            && this->direction_invert_hold == other.direction_invert_hold
            && this->lenth == other.lenth
            && this->manual_radius == other.manual_radius
            && this->max_history_num == other.max_history_num
            && this->max_target_speed == other.max_target_speed
            && this->output_point == other.output_point
            && this->output_target_min_speed == other.output_target_min_speed
            && this->prediction_enabled == other.prediction_enabled
            && this->prediction_width == other.prediction_width
            && this->radius_coeff == other.radius_coeff
            && this->scan_time == other.scan_time
            && this->use_video_radius == other.use_video_radius
            && this->video_overlap_cnt == other.video_overlap_cnt
            && this->head == other.head
            && this->use_original_video_img == other.use_original_video_img
            && this->merge_video == other.merge_video
            && this->video_color_list == other.video_color_list);
}

QJsonValue zchxVideoParse::toJson() const
{
    QJsonObject obj;
    obj.insert("lenth", lenth.toJson());
    obj.insert("area", area.toJson());
    obj.insert("amp", amp.toJson());
    obj.insert("max_history_num", max_history_num);
    obj.insert("clear_target_time", clear_target_time);
    obj.insert("prediction_width", prediction_width);
    obj.insert("video_overlap_cnt", video_overlap_cnt);
    obj.insert("max_target_speed", max_target_speed);
    obj.insert("scan_time", scan_time);
    obj.insert("radius_coeff", radius_coeff);
    obj.insert("use_video_radius", use_video_radius);
    obj.insert("direction_invert_hold", direction_invert_hold);
    obj.insert("adjust_cog", adjust_cog);
    obj.insert("confirm_target_cnt", confirm_target_cnt);
    obj.insert("check_target_gap", check_target_gap);
    obj.insert("output_point", output_point);
    obj.insert("output_target_min_speed", output_target_min_speed);
    obj.insert("prediction_enabled", prediction_enabled);
    obj.insert("manual_radius", manual_radius);
    obj.insert("head", head);
    obj.insert("use_original_video_img", use_original_video_img);
    obj.insert("merge_video", merge_video);
    obj.insert("video_color_list", QJsonArray::fromVariantList(video_color_list));
    return obj;
}


zchxRadarDevice::zchxRadarDevice(const QJsonObject& obj) : zchxData(obj),
    base(obj.value("base").toObject()),
    channel_list(obj.value("channel").toArray()),
    parse_param(obj.value("video_parse").toObject()),
    local_connect_ip(obj.value("local_connect_ip").toString())
{

}

QJsonValue zchxRadarDevice::toJson() const
{
    QJsonObject obj;
    obj.insert("base", base.toJson());
    obj.insert("channel", channel_list.toJson());
    obj.insert("video_parse", parse_param.toJson());
    obj.insert("local_connect_ip", local_connect_ip);
    return obj;
}

zchxRadarServerCfg::zchxRadarServerCfg(const QJsonObject& obj) : zchxData(obj)
{
    app = obj.value("app").toString();
    version = obj.value("version").toString();
    date = obj.value("date").toString();
    server = zchxSocket(obj.value("server").toObject());
    publish_list = zchxPublishSettingsList(obj.value("publish_list").toArray());
    device_list = zchxRadarDeviceList(obj.value("device_list").toArray());
    filter_enabled = obj.value("filter_enabled").toBool();
    debug_output = false;
    if(obj.contains("debug_output"))
    {
        debug_output = obj.value("debug_output").toBool();
    }
}

QJsonValue  zchxRadarServerCfg::toJson() const
{
    QJsonObject obj;
    obj.insert("app", app);
    obj.insert("version", version);
    obj.insert("date", date);
    obj.insert("server", server.toJson());
    obj.insert("publish_list", publish_list.toJson());
    obj.insert("device_list", device_list.toJson());
    obj.insert("filter_enabled", filter_enabled);
    obj.insert("debug_output", debug_output);
    return obj;
}

zchxPortStatus::zchxPortStatus(const QJsonObject& obj) : zchxData(obj)
{
    port = obj.value("port").toInt();
    sts = obj.value("sts").toBool();
    topic = obj.value("topic").toString();
}

QJsonValue zchxPortStatus::toJson() const
{
    QJsonObject obj;
    obj.insert("port", port);
    obj.insert("sts", sts);
    obj.insert("topic", topic);
}


zchxRadarCtrlDef::zchxRadarCtrlDef() : zchxData()
{
    radar_id = 0;
    ch_id = 0;
    jsval = QJsonValue();
    type = 0;
}

zchxRadarCtrlDef::zchxRadarCtrlDef(const QJsonObject& obj) : zchxData(obj)
{
    radar_id = obj.value("radar_id").toInt();
    ch_id = obj.value("ch_id").toInt();
    jsval = obj.value("jsval");
    type = obj.value("type").toInt();
}

QJsonValue zchxRadarCtrlDef::toJson() const
{
    QJsonObject obj;
    obj.insert("radar_id", radar_id);
    obj.insert("ch_id", ch_id);
    obj.insert("jsval", jsval);
    obj.insert("type", type);
    return obj;
}

zchxRadarReportData::zchxRadarReportData(const QJsonObject &obj)
{
    timeOfDay = obj.value("timeOfDay").toVariant().toLongLong();
    type = obj.value("type").toInt();
    jsval = obj.value("jsval");
}

QJsonValue zchxRadarReportData::toJson() const
{
    QJsonObject obj;
    obj.insert("timeOfDay", timeOfDay);
    obj.insert("type", type);
    obj.insert("jsval", jsval);
    return obj;
}

zchxRadarChannelReport::zchxRadarChannelReport(const QJsonObject& obj) : zchxData(obj)
{
    mRadarID = obj.value("radar_id").toInt();
    mChannelID = obj.value("ch_id").toInt();
    time = obj.value("time").toString();
    reportList = zchxRadarReportDataList(obj.value("report").toArray());
}


QJsonValue zchxRadarChannelReport::toJson() const
{
    QJsonObject obj;
    obj.insert("radar_id", mRadarID);
    obj.insert("ch_id", mChannelID);
    obj.insert("time", time);
    obj.insert("report", reportList.toJson());
    return obj;
}

zchxRadarDeviceBaseSetting::zchxRadarDeviceBaseSetting(const QJsonObject &obj) :
    zchxData(obj),
    center(obj.value("center").toArray())
{
    id = obj.value("id").toInt();
    name = obj.value("name").toString();
}

QJsonValue zchxRadarDeviceBaseSetting::toJson() const
{
    QJsonObject obj;
    obj.insert("id", id);
    obj.insert("name", name);
    obj.insert("center", center.toJson());
    return obj;
}


