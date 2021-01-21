#ifndef ZCHXDATADDEF_H
#define ZCHXDATADDEF_H

#include "zchxbasedata.h"
#define     JSON_CMD            "cmd"
#define     JSON_CMD_STR        "cmd_str"
#define     JSON_VAL            "content"
#define     JSON_STATUS         "status"
#define     JSON_STATUS_STR     "status_str"
#define     JSON_REQUEST_TIME   "request_time"
#define     JSON_OK             1
#define     JSON_ERROR          0

namespace zchxCommon{
enum MsgCmd{
    Msg_Undefined = 0,
    Msg_Edit_FilterArea = 1,
    Msg_Delete_FilterArea,
    Msg_Heart,
    Msg_Request_Cfg,
    Msg_Update_PublishSetting,
    Msg_Update_RadarBaseSetting,
    Msg_Update_ChannelSetting,
    Msg_Update_ParseSetting,
    Msg_Delete_Radar,
    Msg_Update_RadarCtrl,
    Msg_Update_BaseAndChannel,
};

enum  THREAD_TYPE{
    THREAD_DATA_REQREP = 0,
    THREAD_DATA_RADAR_TRACK,
    THREAD_DATA_RADAR_VIDEO,
//    THREAD_DATA_FILTER,
    THREAD_DATA_RADAR_REPORT,
    THREAD_DATA_AIS,
    THREAD_DATA_SVRCFG,
};

enum SVRCFG_TYPE{
    SVR_UPDATE_PUBLISH_SETTING = 0,
    SVR_UPDATE_RADAR_DEV,
    SVR_UPDATE_RADAR_FILTER,
    SVR_UPDATE_PUBLISH_PORT_STATUS,
    SVR_UPDATE_RADAR_TYPE,
    SVR_UPDATE_RADAR_RADIUS,
};




//添加数据结构
class COMMONSHARED_EXPORT zchxLatlon : public zchxData
{
public:
    zchxLatlon() : zchxData() {lat = 0.0; lon = 0.0;}
    zchxLatlon(double lat_, double lon_) : zchxData() {lat = lat_; lon = lon_;}
    zchxLatlon(const QJsonArray& array);
    zchxLatlon(const QJsonObject& obj) : zchxData(obj) {}
    QJsonValue toJson() const;
public:
    double lat;
    double lon;
};


class COMMONSHARED_EXPORT zchxArea : public zchxDataList<zchxLatlon>
{
public:
    zchxArea() : zchxDataList() {}
    zchxArea(const zchxLatlon& ll) : zchxDataList<zchxLatlon>(ll) {}
    zchxArea(const QJsonArray& array) : zchxDataList<zchxLatlon>(array) {}
public:
};


class COMMONSHARED_EXPORT zchxFilterArea : public zchxData
{
public:
    int             type;           //目标在区域内是过滤还是保留。0；过滤；1保留
    int             id;
    zchxArea        area;
    qint64          time;
    QString         name;

public:
    zchxFilterArea() : zchxData() {id = 0; type = 0; time = 0; name = "";}
    zchxFilterArea(const QJsonObject& obj);
    zchxFilterArea(const QJsonArray& array) : zchxData(array) {}
    QJsonValue toJson() const;
};



class COMMONSHARED_EXPORT zchxfilterAreaList : public zchxDataList<zchxFilterArea>
{
public:
    zchxfilterAreaList() : zchxDataList<zchxFilterArea>(){}
    zchxfilterAreaList(const QJsonArray& array) : zchxDataList<zchxFilterArea>(array) {}
    zchxfilterAreaList(const zchxFilterArea& area) : zchxDataList<zchxFilterArea>(area) {}
    zchxfilterAreaList(const QList<zchxFilterArea>& list) : zchxDataList<zchxFilterArea>(list) {}
};


class COMMONSHARED_EXPORT zchxPublishSetting : public zchxData
{
public:
    int         id;
    QString     topic;
    int         port;
    QString     desc;
    bool        status;

public:
    zchxPublishSetting():zchxData() {id = 0, topic = ""; port = 0; desc = ""; status = false;}
    zchxPublishSetting(int _id, const QString& topic_p, int port_p, const QString& desc_p = QString());
    zchxPublishSetting(const QJsonObject& obj);
    zchxPublishSetting(const QJsonArray& array) : zchxData(array) {}
    QJsonValue toJson() const;
};



class COMMONSHARED_EXPORT zchxPublishSettingsList : public zchxDataList<zchxPublishSetting>
{
public:
    zchxPublishSettingsList() : zchxDataList<zchxPublishSetting>(){}
    zchxPublishSettingsList(const QJsonArray& array) : zchxDataList<zchxPublishSetting>(array) {}
    zchxPublishSettingsList(const zchxPublishSetting& area) : zchxDataList<zchxPublishSetting>(area) {}
};

class COMMONSHARED_EXPORT zchxSocket : public zchxData
{
public:
    QString ip;
    int port;
    QString desc;
    bool status;
public:
    zchxSocket():zchxData(){ip = ""; port = 0; desc = ""; status = false;}
    zchxSocket(const QString& s, int i, const QString& d = QString());
    zchxSocket(const QJsonObject& obj);
    zchxSocket(const QJsonArray& array) : zchxData(array) {}
    QJsonValue toJson() const;
};

class COMMONSHARED_EXPORT zchxSocketList : public zchxDataList<zchxSocket>
{
public:
    zchxSocketList() : zchxDataList<zchxSocket>(){}
    zchxSocketList(const QJsonArray& array) : zchxDataList<zchxSocket>(array) {}
    zchxSocketList(const zchxSocket& area) : zchxDataList<zchxSocket>(area) {}
};


class COMMONSHARED_EXPORT zchxRadarChannel : public zchxData
{
public:
    int             id;
    zchxSocket      video;
    zchxSocket      report;
    zchxSocket      heart;
    int             scanline;       //扫描线数
    int             cell;           //单元数

public:
    zchxRadarChannel();
    zchxRadarChannel(const QJsonObject& obj);
    zchxRadarChannel(const QJsonArray& array) : zchxData(array) {}
    bool operator ==(const zchxRadarChannel& other) const { return this->id == other.id;}
    QJsonValue toJson() const;
};

class COMMONSHARED_EXPORT zchxRadarChannelList : public zchxDataList<zchxRadarChannel>
{
public:
    zchxRadarChannelList() : zchxDataList<zchxRadarChannel>(){}
    zchxRadarChannelList(const QJsonArray& array) : zchxDataList<zchxRadarChannel>(array) {}
    zchxRadarChannelList(const zchxRadarChannel& area) : zchxDataList<zchxRadarChannel>(area) {}
};



enum RadarType{
    RADAR_UNKNOWN = 0,
    RADAR_BR24,
    RADAR_3G,
    RADAR_4G,
    RADAR_6G,
};

enum RangeCheckType{
    Check_Length = 1,
    Check_Area,
    Check_Video_Amp,
};

class COMMONSHARED_EXPORT zchxRangeCheck : public zchxData
{
public:
    int     id;
    int     min;
    int     max;
    bool    used;

public:
    zchxRangeCheck() : zchxData() {used = true; id = -1; min = 0; max = 0;}
    zchxRangeCheck(const QJsonArray& array) : zchxData(array) {}
    zchxRangeCheck(const QJsonObject& obj);
    QJsonValue toJson() const;
    bool operator ==(const zchxRangeCheck& other) const
    {
        return id == other.id && min == other.min && max == other.max && used == other.used;
    }
};

class COMMONSHARED_EXPORT zchxRangeCheckList : public zchxDataList<zchxRangeCheck>
{
public:
    zchxRangeCheckList() : zchxDataList<zchxRangeCheck>(){}
    zchxRangeCheckList(const QJsonArray& array) : zchxDataList<zchxRangeCheck>(array) {}
    zchxRangeCheckList(const zchxRangeCheck& area) : zchxDataList<zchxRangeCheck>(area) {}
};



class COMMONSHARED_EXPORT zchxVideoParse : public zchxData
{
public:
    zchxRangeCheck      lenth;
    zchxRangeCheck      area;
    zchxRangeCheck      amp;
    int             max_history_num;
    int             clear_target_time;
    int             prediction_width;
    bool            prediction_enabled;
    int             video_overlap_cnt;
    bool            merge_video;
    QVariantList     video_color_list;
    int             max_target_speed;
    double          scan_time;
    double          manual_radius;
    double          radius_coeff;
    bool            use_video_radius;
    double          direction_invert_hold;
    bool            adjust_cog;
    int             confirm_target_cnt;
    bool            check_target_gap;
    bool            output_point;
    double          output_target_min_speed;
    int             head;
    bool            use_original_video_img;

public:
    zchxVideoParse();
    zchxVideoParse(const QJsonObject& obj);
    zchxVideoParse(const QJsonArray& array) : zchxData(array) {}
    QJsonValue toJson() const;
    bool operator ==(const zchxVideoParse& other) const;
};


class COMMONSHARED_EXPORT zchxRadarDeviceBaseSetting : public zchxData
{
public:
    int                             id;
    QString                         name;
    zchxLatlon                      center;
public:
    zchxRadarDeviceBaseSetting() : zchxData() {id = -1;}
    zchxRadarDeviceBaseSetting(const QJsonArray& array) : zchxData(array) {}
    zchxRadarDeviceBaseSetting(const QJsonObject& obj);
    QJsonValue toJson() const;
};



class COMMONSHARED_EXPORT zchxRadarDevice : public zchxData
{
public:
    zchxRadarDeviceBaseSetting      base;
    zchxRadarChannelList            channel_list;
    zchxVideoParse                  parse_param;
    QString                         local_connect_ip;
public:
    zchxRadarDevice() : zchxData() {}
    zchxRadarDevice(const QJsonArray& array) : zchxData(array) {}
    zchxRadarDevice(const QJsonObject& obj);
    QJsonValue toJson() const;
};

class COMMONSHARED_EXPORT zchxRadarDeviceList : public zchxDataList<zchxRadarDevice>
{
public:
    zchxRadarDeviceList() : zchxDataList<zchxRadarDevice>(){}
    zchxRadarDeviceList(const QJsonArray& array) : zchxDataList<zchxRadarDevice>(array) {}
    zchxRadarDeviceList(const zchxRadarDevice& area) : zchxDataList<zchxRadarDevice>(area) {}
};


class COMMONSHARED_EXPORT zchxRadarServerCfg : public zchxData
{
public:
    QString                                 app;
    QString                                 version;
    QString                                 date;
    zchxSocket                              server;
    zchxPublishSettingsList                 publish_list;
    zchxRadarDeviceList                     device_list;
    bool                                    filter_enabled;
    bool                                    debug_output;

    zchxRadarServerCfg() : zchxData() { filter_enabled = true; debug_output = false;}
    zchxRadarServerCfg(const QJsonArray& array) : zchxData(array) {}
    zchxRadarServerCfg(const QJsonObject& obj);
    QJsonValue  toJson() const;
};


class COMMONSHARED_EXPORT zchxPortStatus : public zchxData
{
public:
    int     port;
    bool    sts;
    QString topic;

public:
    zchxPortStatus() : zchxData() {sts = false;}
    zchxPortStatus(const QJsonArray& array) : zchxData(array) {}
    zchxPortStatus(const QJsonObject& obj);
    QJsonValue  toJson() const;
    QString    toString() const
    {
        return QString("Port: %1 Status: %2  Tpoic:%3").arg(port).arg(sts == true ? "ON" : "OFF").arg(topic);
    }

};

class COMMONSHARED_EXPORT zchxPortStatusList : public zchxDataList<zchxPortStatus>
{
public:
    zchxPortStatusList() : zchxDataList<zchxPortStatus>(){}
    zchxPortStatusList(const QJsonArray& array) : zchxDataList<zchxPortStatus>(array) {}
    zchxPortStatusList(const zchxPortStatus& area) : zchxDataList<zchxPortStatus>(area) {}
};



enum RadarObjType{
    RadarPointUndef = 0,        //未知
    RadarPointNormal,
    RadarPointBarrier,          //障碍物
    RadarPointBuyo,             //浮标
    RadarPointFishRaft,         //渔排
    RadarPointSpecialShip,      //特定船舶
    RadarPointPerson,           //人
    RadarPointCar,              //车
    RadarPointShip,             //一般船舶
};


struct UserSpecifiedObj
{
    int     mTrackNum;
    int     mType;
    QString mName;
};

//------------------------------------------------------------------------------------------------
// Info Type 消息类型
//------------------------------------------------------------------------------------------------
//-|num |-------------Status Discription--------------------|------------中文-------------------
// | 1  | Power                                             |  雷达电源控制
// | 2  | Scan speed                                        |  扫描速度
// | 3  | Antenna height                                    |  天线高度
// | 4  | Bearing alignment                                 |  方位校准
// | 5  | Rang                                              |  半径
// | 6  | Gain                                              |  增益
// | 7  | Sea clutter                                       |  海杂波
// | 8  | Rain clutter                                      |  雨杂波
// | 9  | Noise rejection                                   |  噪声抑制
// | 10 | Side lobe suppression                             |  旁瓣抑制
// | 11 | Interference rejection                            |  抗干扰
// | 12 | Local interference rejection                      |  本地抗干扰
// | 13 | Target expansion                                  |  目标扩展
// | 14 | Target boost                                      |  目标推进
// | 15 | Target separation                                 |  目标分离
//------------------------------------------------------------------------------------------------

enum INFOTYPE {
    UNKNOWN 									= 0,
    POWER                               		= 1,
    SCAN_SPEED                           		= 2,
    ANTENNA_HEIGHT								= 3,
    BEARING_ALIGNMENT							= 4,
    RANG                          				= 5,
    GAIN                           				= 6,
    SEA_CLUTTER                   				= 7,
    RAIN_CLUTTER                     			= 8,
    NOISE_REJECTION                           	= 9,
    SIDE_LOBE_SUPPRESSION                      	= 10,
    INTERFERENCE_REJECTION	             		= 11,
    LOCAL_INTERFERENCE_REJECTION             	= 12,
    TARGET_EXPANSION  							= 13,
    TARGET_BOOST                   				= 14,
    TARGET_SEPARATION                     		= 15,    
    RESET                                       = 16,
    FAN_IGNORE                                  = 17,
    OUTPUT_DATA,
    OPEN,
    RESVERED,
};

enum VALUEUNIT {
    UNIT_UNKNOWN										= 1,
    UNIT_BOOL                               				= 2,
    UNIT_DEGREE                           				= 3,
    UNIT_METER											= 4,
};

#define         STR_MODE_CHI            0
#define         STR_MODE_ENG            1

struct zchxFanSector{
    bool    is_open;
    int     start;
    int     end;
    int     sector;

    zchxFanSector()
    {
        is_open = false;
        start = 0;
        end = 0;
        sector = 0;
    }
};

class COMMONSHARED_EXPORT zchxRadarCtrlDef : zchxData
{
public:
    int                 radar_id;					//消息类型
    int     			ch_id;               //值类型
    QJsonValue                 jsval;          //这里主要是扇形匿形的命令传入的是多个值，所以需要将传入值再次进行封装
    int                 type;
public:
    zchxRadarCtrlDef();
    zchxRadarCtrlDef(const QJsonArray& array) : zchxData(array) {}
    zchxRadarCtrlDef(const QJsonObject& obj);
    QJsonValue toJson() const;
};

class COMMONSHARED_EXPORT zchxRadarCtrlDefList : public zchxDataList<zchxRadarCtrlDef>
{
public:
    zchxRadarCtrlDefList() : zchxDataList<zchxRadarCtrlDef>(){}
    zchxRadarCtrlDefList(const QJsonArray& array) : zchxDataList<zchxRadarCtrlDef>(array) {}
    zchxRadarCtrlDefList(const zchxRadarCtrlDef& area) : zchxDataList<zchxRadarCtrlDef>(area) {}
};



class COMMONSHARED_EXPORT zchxRadarReportData : zchxData
{
public:
    qint64 				timeOfDay;			// 当日时间
    int      			type;				// 消息类型
    QJsonValue	  			    jsval;				// 设置值
public:
    zchxRadarReportData() : zchxData() {}
    zchxRadarReportData(const QJsonArray& array) : zchxData(array) {}
    zchxRadarReportData(const QJsonObject& obj);
    QJsonValue toJson() const;

};

class COMMONSHARED_EXPORT zchxRadarReportDataList : public zchxDataList<zchxRadarReportData>
{
public:
    zchxRadarReportDataList() : zchxDataList<zchxRadarReportData>(){}
    zchxRadarReportDataList(const QJsonArray& array) : zchxDataList<zchxRadarReportData>(array) {}
    zchxRadarReportDataList(const zchxRadarReportData& area) : zchxDataList<zchxRadarReportData>(area) {}
};


class COMMONSHARED_EXPORT zchxRadarChannelReport : public zchxData
{
public:
    int                      mRadarID;           //雷达id
    int                      mChannelID;             	// 通道id
    QString                 time;			    // 当日时间
    zchxRadarReportDataList  reportList;		    // 雷达报告信息
public:
    zchxRadarChannelReport() :zchxData() {mRadarID = 0; mChannelID = 0;}
    zchxRadarChannelReport(const QJsonArray& array) : zchxData(array) {}
    zchxRadarChannelReport(const QJsonObject& obj);
    QJsonValue toJson() const;
};

class COMMONSHARED_EXPORT zchxRadarReportList : public zchxDataList<zchxRadarChannelReport>
{
public:
    zchxRadarReportList() : zchxDataList<zchxRadarChannelReport>(){}
    zchxRadarReportList(const QJsonArray& array) : zchxDataList<zchxRadarChannelReport>(array) {}
    zchxRadarReportList(const zchxRadarChannelReport& area) : zchxDataList<zchxRadarChannelReport>(area) {}
    zchxRadarReportList(const QList<zchxRadarChannelReport>& list) : zchxDataList<zchxRadarChannelReport>(list) {}
};


class COMMONSHARED_EXPORT zchxRadarTypeData : public zchxData
{
public :
    QString             radar_id;
    int                 channel_id;
    int                 type;
public:
    zchxRadarTypeData() : zchxData() {radar_id = ""; channel_id = -1; type = -1;}
    zchxRadarTypeData(const QJsonArray& array) : zchxData(array) {}
    zchxRadarTypeData(const QJsonObject& obj) : zchxData(obj)
    {
        radar_id = obj.value("radar_id").toString();
        channel_id = obj.value("channel_id").toInt();
        type = obj.value("type").toInt();
    }
    QJsonValue toJson() const
    {
        QJsonObject obj;
        obj.insert("channel_id", channel_id);
        obj.insert("radar_id", radar_id);
        obj.insert("type", type);
        return obj;
    }

};

}

Q_DECLARE_METATYPE(zchxCommon::zchxLatlon)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarTypeData)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarChannelReport)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarReportList)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarReportData)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarReportDataList)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarCtrlDef)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarCtrlDefList)
Q_DECLARE_METATYPE(zchxCommon::zchxPortStatus)
Q_DECLARE_METATYPE(zchxCommon::zchxPortStatusList)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarChannel)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarChannelList)
Q_DECLARE_METATYPE(zchxCommon::zchxRangeCheck)
Q_DECLARE_METATYPE(zchxCommon::zchxRangeCheckList)
Q_DECLARE_METATYPE(zchxCommon::zchxVideoParse)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarDeviceBaseSetting)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarDevice)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarDeviceList)
Q_DECLARE_METATYPE(zchxCommon::zchxRadarServerCfg)
Q_DECLARE_METATYPE(zchxCommon::zchxPublishSettingsList)
Q_DECLARE_METATYPE(zchxCommon::zchxSocket)
Q_DECLARE_METATYPE(zchxCommon::zchxSocketList)
Q_DECLARE_METATYPE(zchxCommon::zchxPublishSetting)
Q_DECLARE_METATYPE(zchxCommon::zchxArea)
Q_DECLARE_METATYPE(zchxCommon::zchxfilterAreaList)
Q_DECLARE_METATYPE(zchxCommon::zchxFilterArea)
Q_DECLARE_METATYPE(zchxCommon::zchxFanSector)



#endif // ZCHXDATADDEF_H
