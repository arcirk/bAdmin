#ifndef SHARED_STRUCT_HPP
#define SHARED_STRUCT_HPP

#include <iostream>
#include <exception>
#include <functional>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <nlohmann/json.hpp>
#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>
#include <vector>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>

#define ARCIRK_VERSION "1.1.0"
#define CONF_FILENAME "b_admin_conf.json"
#define CLIENT_VERSION 2

#define FAT12          "FAT12"
#define REGISTRY       "REGISTRY"
#define HDIMAGE        "HDIMAGE"
#define DATABASE       "DATABASE"
#define REMOTEBASE     "REMOTEBASE"

#define CRYPT_KEY "my_key"

#define APP_NAME "bAdmin"

#define WS_RESULT_SUCCESS "success"
#define WS_RESULT_ERROR "error"

typedef unsigned char BYTE;
typedef std::vector<BYTE> ByteArray;

#define NIL_STRING_UUID "00000000-0000-0000-0000-000000000000"

namespace arcirk{
    template<typename T>
    static inline std::string enum_synonym(T value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    };

    template<typename T>
    nlohmann::json values_from_param(const nlohmann::json& param){
        if(param.empty())
            return {};
        T e = T();
        auto source = pre::json::to_json(e);
        nlohmann::json result = {};

        if(source.is_object()){
            for (auto itr = source.begin(); itr != source.end() ; ++itr) {
                auto i = param.find(itr.key());
                if( i != param.end()){
                    result[itr.key()] = i.value();
                }
            }
            return result;
        }else
            return {};
    }

    static inline std::string type_string(nlohmann::json::value_t t){
        using json = nlohmann::json;
        if(t == json::value_t::null) return "null";
        else if(t == json::value_t::boolean) return "boolean";
        else if(t == json::value_t::number_integer) return "number_integer";
        else if(t == json::value_t::number_unsigned) return "number_unsigned";
        else if(t == json::value_t::number_float) return "number_float";
        else if(t == json::value_t::object) return "object";
        else if(t == json::value_t::array) return "array";
        else if(t == json::value_t::string) return "string";
        else return "undefined";
    }

    template<typename T>
    T secure_serialization(const nlohmann::json &source)
    {
        using json = nlohmann::json;

        if(!source.is_object())
            return T();

        try {
            auto result = pre::json::from_json<T>(source);
            return result;
        }catch (const std::exception& e){
            std::cerr << __FUNCTION__ << e.what() << std::endl;
        }

        nlohmann::json object = pre::json::to_json(T());

        for (auto it = source.items().begin(); it != source.items().end(); ++it) {
            if(object.find(it.key()) != object.end()){
                if(it.value().type() == object[it.key()].type()){
                    object[it.key()] = it.value();
                }else{
                    if(it.value().type() == json::value_t::number_unsigned &&
                            (object[it.key()].type() == json::value_t::number_integer ||
                                    object[it.key()].type() == json::value_t::number_float)){
                        object[it.key()] = it.value();
                    }else{
                        std::cerr << __FUNCTION__ << " Ошибка проверки по типу ключа: " << it.key().c_str() << std::endl;
                        std::cerr << it.value() << " " << type_string(it.value().type()) << " " << type_string(object[it.key()].type()) <<  std::endl;
                    }
                }
            }
        }

        return pre::json::from_json<T>(object);
    }

    template<typename T>
    T secure_serialization(const std::string &source)
    {
        using json = nlohmann::json;
        try {
            return secure_serialization<T>(json::parse(source));
        } catch (std::exception& e) {
            std::cerr << __FUNCTION__ << e.what() << std::endl;
        }
        return T();
    }

    template<typename T>
    T internal_structure(const std::string &name, const nlohmann::json &source)
    {
        auto result = T();
        try {
            auto obg = source.value(name, nlohmann::json::object());
            if(!obg.empty())
                result = pre::json::from_json<T>(obg);
        } catch (const std::exception& e) {
           std::cerr <<  e.what() << std::endl;
        }

        return result;
    }

    namespace uuids{
        inline std::string nil_string_uuid() {return "00000000-0000-0000-0000-000000000000";};
    }
    inline std::string byte_array_to_string(const ByteArray& data){
        return std::string(data.begin(), data.end());
    }

    inline ByteArray string_to_byte_array(const std::string& str){
        return ByteArray(str.begin(), str.end());
    }
    inline void write_file(const std::string& filename, ByteArray& file_bytes){
        std::ofstream file(filename, std::ios::out|std::ios::binary);
        std::copy(file_bytes.cbegin(), file_bytes.cend(),
                  std::ostream_iterator<unsigned char>(file));
    }

    inline void read_file(const std::string &filename, ByteArray &result)
    {

        FILE * fp = fopen(filename.c_str(), "rb");
        if (fp == NULL)
            throw std::runtime_error(std::strerror(errno));

        fseek(fp, 0, SEEK_END);
        size_t flen= ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::vector<unsigned char> v (flen);

        fread(&v[0], 1, flen, fp);

        fclose(fp);

        result = v;
    }

    inline std::string from_utf(const std::string& source){
#ifdef BOOST_WINDOWS
        return boost::locale::conv::from_utf(source, "windows-1251");
#else
        return source;
#endif
    }

    inline std::string to_utf(const std::string& source, const std::string& ch = "windows-1251"){
#ifdef BOOST_WINDOWS
        return boost::locale::conv::to_utf<char>(source, ch);
#else
        return source;
#endif
    }

    inline int index_of(const std::string& original_string, const std::string& source){
        auto find = original_string.find(source);
        if(find == std::string::npos)
            return - 1;
        else
            return (int)find;
    }
}

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::services), task_options,
        (std::string, uuid)
        (std::string, name)
        (std::string, synonum)
        (bool, predefined)
        (int, start_task)
        (int, end_task)
        (int, interval)
        (bool, allowed)
        (std::string, days_of_week)
        (std::string, script)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::synchronize), http_param,
        (std::string, command)
        (std::string, param)    //base64 строка
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), version_application,
        (int, major)
        (int, minor)
        (int, path)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::synchronize), http_query_sql_param,
        (std::string, table_name)
        (std::string, query_type)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), client_param,
        (std::string, app_name)
        (std::string, user_uuid)
        (std::string, user_name)
        (std::string, hash)
        (std::string, host_name)
        (std::string, password)
        (std::string, session_uuid)
        (std::string, system_user)
        (std::string, device_id)
        (std::string, info_base)
        (std::string, product)
        (std::string, sid)
        (int, version)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), session_info,
        (std::string, session_uuid)
        (std::string, user_name)
        (std::string, user_uuid)
        (std::string, start_date)
        (std::string, app_name)
        (std::string, role)
        (std::string, device_id)
        (std::string, address)
        (std::string, info_base)
        (std::string, host_name)
        (std::string, product)
        (std::string, system_user)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), mstsc_options,
        (std::string, name)
        (std::string, address)
        (int, port)
        (bool, def_port)
        (bool, not_full_window)
        (int, width)
        (int, height)
        (bool, reset_user)
        (std::string, user_name)
        (std::string, password)
        (std::string, uuid)
)
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), mpl_options,
        (bool, use_firefox)
        (std::string, firefox_path)
        (std::string, profiles_path)
        (ByteArray, mpl_list)
        (ByteArray, mpl_profiles)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), mpl_item,
        (std::string, profile)
        (std::string, name)
        (std::string, url)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), cryptopro_data,
        (std::string, cryptopro_path)
        (ByteArray, certs)
        (ByteArray, conts)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_response,
        (std::string, command)
        (std::string, message)
        (std::string, param)
        (std::string, result)
        (std::string, sender)
        (std::string, receiver)
        (std::string, uuid_form)
        (std::string, app_name)
        (std::string, sender_name)
        (std::string, sender_uuid)
        (std::string, receiver_name)
        (std::string, receiver_uuid)
        (std::string, version)
        (ByteArray, data)
)


namespace arcirk::server{

    enum server_commands{
        ServerVersion, //Версия сервера
        ServerOnlineClientsList, //Список активных пользователей
        SetClientParam, //Параметры клиента
        ServerConfiguration, //Конфигурация сервера
        UserInfo, //Информация о пользователе (база данных)
        InsertOrUpdateUser, //Обновить или добавить пользователя (база данных)
        CommandToClient, //Команда клиенту (подписчику)
        ServerUsersList, //Список пользователей (база данных)
        ExecuteSqlQuery, //выполнить запрос к базе данных
        GetMessages, //Список сообщений
        UpdateServerConfiguration, //Обновить конфигурацию сервера
        HttpServiceConfiguration, //Получить конфигурацию http сервиса 1С
        InsertToDatabaseFromArray, //Добавить массив записей в базу //устарела удалить
        SetNewDeviceId, //Явная установка идентификатора на устройствах где не возможно его получить
        ObjectSetToDatabase, //Синхронизация объекта 1С с базой
        ObjectGetFromDatabase, //Получить объект типа 1С из базы данных для десериализации
        SyncGetDiscrepancyInData, //Получить расхождения в данных между базами на клиенте и на Сервере
        SyncUpdateDataOnTheServer, //Обновляет данные на сервере по запросу клиента
        WebDavServiceConfiguration, //Получить настройки WebDav
        SyncUpdateBarcode, //синхронизирует на сервере штрихкод и номенклатуру по запросу клиента с сервером 1с
        DownloadFile, //Загружает файл на сервер
        GetInformationAboutFile, //получить информацию о файле
        CheckForUpdates, //поиск фалов обрновления
        UploadFile, //скачать файл
        GetDatabaseTables,
        FileToDatabase, //Загрузка таблицы базы данных из файла
        ProfileDirFileList,
        ProfileDeleteFile,
        DeviceGetFullInfo,
        GetTasks,
        UpdateTaskOptions,
        TasksRestart,
        RunTask,
        StopTask,
        CMD_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_commands, {
        {CMD_INVALID, nullptr}    ,
        {ServerVersion, "ServerVersion"}  ,
        {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,
        {SetClientParam, "SetClientParam"}    ,
        {ServerConfiguration, "ServerConfiguration"}    ,
        {UserInfo, "UserInfo"}    ,
        {InsertOrUpdateUser, "InsertOrUpdateUser"}    ,
        {CommandToClient, "CommandToClient"}    ,
        {ServerUsersList, "ServerUsersList"}    ,
        {ExecuteSqlQuery, "ExecuteSqlQuery"}    ,
        {GetMessages, "GetMessages"}    ,
        {UpdateServerConfiguration, "UpdateServerConfiguration"}    ,
        {HttpServiceConfiguration, "HttpServiceConfiguration"}    ,
        {InsertToDatabaseFromArray, "InsertToDatabaseFromArray"}    ,
        {SetNewDeviceId, "SetNewDeviceId"}    ,
        {ObjectSetToDatabase, "ObjectSetToDatabase"}    ,
        {ObjectGetFromDatabase, "ObjectGetFromDatabase"}    ,
        {SyncGetDiscrepancyInData, "SyncGetDiscrepancyInData"}    ,
        {SyncUpdateDataOnTheServer, "SyncUpdateDataOnTheServer"}    ,
        {WebDavServiceConfiguration, "WebDavServiceConfiguration"}    ,
        {SyncUpdateBarcode, "SyncUpdateBarcode"}    ,
        {DownloadFile, "DownloadFile"}    ,
        {GetInformationAboutFile, "GetInformationAboutFile"}    ,
        {CheckForUpdates, "CheckForUpdates"}    ,
        {UploadFile, "UploadFile"}    ,
        {GetDatabaseTables, "GetDatabaseTables"}    ,
        {FileToDatabase, "FileToDatabase"}    ,
        {ProfileDirFileList, "ProfileDirFileList"}    ,
        {ProfileDeleteFile, "ProfileDeleteFile"}    ,
        {DeviceGetFullInfo, "DeviceGetFullInfo"}    ,
        {GetTasks, "GetTasks"}    ,
        {UpdateTaskOptions, "UpdateTaskOptions"}    ,
        {TasksRestart, "TasksRestart"}    ,
        {RunTask, "RunTask"}    ,
        {StopTask, "StopTask"}    ,
    });

    enum server_objects{
        OnlineUsers,
        Root,
        Services,
        Database,
        DatabaseTables,
        Devices,
        DatabaseUsers,
        ProfileDirectory,
        CertManager,
        Containers,
        Certificates,
        CertUsers,
        LocalhostUser,
        LocalhostUserCertificates,
        LocalhostUserContainers,
        LocalhostUserContainersRegistry,
        LocalhostUserContainersVolume,
        OBJ_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_objects, {
        {OBJ_INVALID, nullptr} ,
        {OnlineUsers, "OnlineUsers"} ,
        {Root, "Root"} ,
        {Services, "Services"} ,
        {Database, "Database"} ,
        {DatabaseTables, "DatabaseTables"},
        {Devices, "Devices"},
        {DatabaseUsers, "DatabaseUsers"},
        {ProfileDirectory, "ProfileDirectory"},
        {CertManager, "CertManager"},
        {Containers, "Containers"},
        {Certificates, "Certificates"},
        {CertUsers, "CertUsers"},
        {LocalhostUser, "LocalhostUser"},
        {LocalhostUserCertificates, "LocalhostUserCertificates"},
        {LocalhostUserContainers, "LocalhostUserContainers"},
        {LocalhostUserContainersRegistry, "LocalhostUserContainersRegistry"},
        {LocalhostUserContainersVolume, "LocalhostUserContainersVolume"},
    });

    enum application_names{
        PriceChecker,
        ServerManager,
        ExtendedLib,
        APP_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(application_names, {
        {APP_INVALID, nullptr} ,
        {PriceChecker, "PriceChecker"} ,
        {ServerManager, "ServerManager"} ,
        {ExtendedLib, "ExtendedLib"} ,
    });

    enum device_types{
        devDesktop,
        devServer,
        devPhone,
        devTablet,
        devExtendedLib,
        DEV_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(device_types, {
       {DEV_INVALID, nullptr} ,
       {devDesktop, "Desktop"} ,
       {devServer, "Server"} ,
       {devPhone, "Phone"} ,
       {devTablet, "Tablet"} ,
       {devExtendedLib, "ExtendedLib"},
   });
}

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), client_conf,
        (std::string, app_name)
        (bool, is_auto_connect)
        (std::string, server_host)
        (std::string, user_name)
        (std::string, hash)
        (std::string, device_id)
        (ByteArray, servers)
        (std::string, price_checker_repo)
        (std::string, server_repo)
        (bool, use_sid)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_config,
        (std::string, ServerHost)
        (int, ServerPort)
        (std::string, ServerUser)
        (std::string, ServerUserHash)
        (std::string, ServerName)
        (std::string, ServerHttpRoot)
        (std::string, ServerWorkingDirectory)
        (bool, AutoConnect)
        (bool, UseLocalWebDavDirectory)
        (std::string, LocalWebDavDirectory)
        (std::string, WebDavHost)
        (std::string, WebDavUser)
        (std::string, WebDavPwd)
        (std::string, WebDavRoot)
        (bool, WebDavSSL)
        (int, SQLFormat)
        (std::string, SQLHost)
        (std::string, SQLUser)
        (std::string, SQLPassword)
        (std::string, HSHost)
        (std::string, HSUser)
        (std::string, HSPassword)
        (bool, ServerSSL)
        (std::string, SSL_crt_file)
        (std::string, SSL_key_file)
        (bool, UseAuthorization)
        (std::string, ApplicationProfile)
        (int, ThreadsCount)
        (std::string, Version)
        (bool, ResponseTransferToBase64)
        (bool, AllowDelayedAuthorization)
        (bool, AllowHistoryMessages)
        (std::string, ExchangePlan)
        (std::string, ServerProtocol)
        (bool, WriteJournal)
        (bool, AllowIdentificationByWINSID)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), user_info,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, hash)
        (std::string, role)
        (std::string, performance)
        (std::string, parent)
        (std::string, cache)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), messages,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, message)
        (std::string, token)
        (int, date)
        (std::string, content_type)
        (int, unread_messages)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), organizations,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), subdivisions,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), warehouses,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), price_types,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), workplaces,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, server)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), devices,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, deviceType)
        (std::string, address)
        (std::string, workplace)
        (std::string, price_type)
        (std::string, warehouse)
        (std::string, subdivision)
        (std::string, organization)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), devices_view,
        (std::string, ref)
        (std::string, first)
        (std::string, second)
        (std::string, device_type)
        (std::string, workplace)
        (std::string, price)
        (std::string, warehouse)
        (std::string, subdivision)
        (std::string, organization)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), documents,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, number)
        (int, date)
        (std::string, xml_type)
        (int, version)
        (std::string, device_id)
        (std::string, workplace)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), document_table,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (double, price)
        (double, quantity)
        (std::string, barcode)
        (std::string, product)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), nomenclature,
        (int, _id)
        (std::string, first) // Наименование
        (std::string, second)
        (std::string, ref)
        (std::string, cache) // Все остальные реквизиты
        (std::string, parent)
        (std::string, vendor_code)
        (std::string, trademark)
        (std::string, unit)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), barcodes,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, barcode)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), table_info_sqlite,
        (int, cid)
        (std::string, name)
        (std::string, type)
        (int, notnull)
        (std::string, dflt_value)
        (int, bk)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), containers,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (ByteArray, data)
        (std::string, subject)
        (std::string, issuer)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (std::string, parent_user)
        (std::string, sha1)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), certificates,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (ByteArray, data)
        (std::string, private_key)
        (std::string, subject)
        (std::string, issuer)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (std::string, parent_user)
        (std::string, serial)
        (std::string, suffix)
        (std::string, sha1)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), cert_users,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, uuid)
        (std::string, sid)
        (std::string, system_user)
        (std::string, host)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

namespace arcirk::database {
    enum tables{
        tbUsers,
        tbMessages,
        tbOrganizations,
        tbSubdivisions,
        tbWarehouses,
        tbPriceTypes,
        tbWorkplaces,
        tbDevices,
        tbDevicesType,
        tbDocuments,
        tbDocumentsTables,
        tbNomenclature,
        tbDatabaseConfig,
        tbBarcodes,
        tbDocumentsMarkedTables,
        tbCertificates,
        tbCertUsers,
        tbContainers,
        tables_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(tables, {
        {tables_INVALID, nullptr}    ,
        {tbUsers, "Users"}  ,
        {tbMessages, "Messages"}  ,
        {tbOrganizations, "Organizations"}  ,
        {tbSubdivisions, "Subdivisions"}  ,
        {tbWarehouses, "Warehouses"}  ,
        {tbPriceTypes, "PriceTypes"}  ,
        {tbWorkplaces, "Workplaces"}  ,
        {tbDevices, "Devices"}  ,
        {tbDevicesType, "DevicesType"}  ,
        {tbDocuments, "Documents"}  ,
        {tbDocumentsTables, "DocumentsTables"}  ,
        {tbNomenclature, "Nomenclature"}  ,
        {tbDatabaseConfig, "DatabaseConfig"}  ,
        {tbBarcodes, "Barcodes"}  ,
        {tbDocumentsMarkedTables, "DocumentsMarkedTables"}  ,
        {tbCertificates, "Certificates"}  ,
        {tbCertUsers, "CertUsers"}  ,
        {tbContainers, "Containers"}  ,
    })

    static inline nlohmann::json table_default_json(arcirk::database::tables table) {

          //using namespace arcirk::database;
          switch (table) {
              case tbUsers:{
                  auto usr_info = user_info();
                  usr_info.ref = arcirk::uuids::nil_string_uuid();
                  usr_info.parent = arcirk::uuids::nil_string_uuid();
                  usr_info.is_group = 0;
                  usr_info.deletion_mark = 0;
                  return pre::json::to_json(usr_info);
              }
              case tbMessages:{
                  auto tbl = messages();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.content_type ="Text";
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
                  //std::string tbl_json = to_string(pre::json::to_json(tbl));
                  //return tbl_json;
              }
              case tbOrganizations:{
                  auto tbl = organizations();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbSubdivisions:{
                  auto tbl = subdivisions();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbWarehouses:{
                  auto tbl = warehouses();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbPriceTypes:{
                  auto tbl = price_types();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbWorkplaces:{
                  auto tbl = workplaces();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.server = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbDevices:{
                  auto tbl = devices();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.deviceType = "Desktop";
                  tbl.address = "127.0.0.1";
                  tbl.workplace = arcirk::uuids::nil_string_uuid();
                  tbl.price_type = arcirk::uuids::nil_string_uuid();
                  tbl.warehouse = arcirk::uuids::nil_string_uuid();
                  tbl.subdivision = arcirk::uuids::nil_string_uuid();
                  tbl.organization = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbDocumentsTables: {
                  auto tbl = document_table();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.price = 0;
                  tbl.quantity = 0;
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbDocumentsMarkedTables: {
//                  auto tbl = document_marked_table();
//                  tbl.ref = arcirk::uuids::nil_string_uuid();
//                  tbl.quantity = 1;
//                  tbl.document_ref = arcirk::uuids::nil_string_uuid();
//                  tbl.parent = arcirk::uuids::nil_string_uuid();
//                  tbl.is_group = 0;
//                  tbl.deletion_mark = 0;
//                  return pre::json::to_json(tbl);
                    break;
              }
              case tbDocuments: {
//                  auto tbl = documents();
//                  tbl.ref = arcirk::uuids::nil_string_uuid();
//                  tbl.device_id = arcirk::uuids::nil_string_uuid();
//                  tbl.date = date_to_seconds();
//                  tbl.parent = arcirk::uuids::nil_string_uuid();
//                  tbl.is_group = 0;
//                  tbl.deletion_mark = 0;
//                  return pre::json::to_json(tbl);
                    break;
              }
              case tbNomenclature: {
                  auto tbl = nomenclature();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
              }
              case tbBarcodes: {
                  auto tbl = barcodes();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  return pre::json::to_json(tbl);
              }
              case tbDatabaseConfig: {
//                  auto tbl = database_config();
//                  tbl.ref = arcirk::uuids::nil_string_uuid();
//                  tbl.version = 0;
//                  return pre::json::to_json(tbl);
                    break;
              }
              case tbCertificates: {
                  auto tbl = certificates();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  tbl._id = 0;
                  return pre::json::to_json(tbl);
              }
              case tbCertUsers: {
                  auto tbl = cert_users();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  return pre::json::to_json(tbl);
              }
              case tbContainers: {
                  auto tbl = containers();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  return pre::json::to_json(tbl);
              }
              case tables_INVALID:{
                  break;
              }
              case tbDevicesType:
                  //return devices_type();
                break;
          }

          return {};
      }

    template<typename T>
    static inline T table_default_struct(arcirk::database::tables table){
        auto j = table_default_json(table);
        auto result = pre::json::from_json<T>(j);
        return result;
    }

}

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))
inline void* _crypt(void* data, unsigned data_size, void* key, unsigned key_size)
{
    assert(data && data_size);
    if (!key || !key_size) return data;

    auto* kptr = (uint8_t*)key; // начало ключа
    uint8_t* eptr = kptr + key_size; // конец ключа

    for (auto* dptr = (uint8_t*)data; data_size--; dptr++)
    {
        *dptr ^= *kptr++;
        if (kptr == eptr) kptr = (uint8_t*)key; // переход на начало ключа
    }
    return data;
}

namespace arcirk {
    static inline std::string crypt(const std::string &source, const std::string& key) {

#ifdef _WINDOWS
        std::string s  = arcirk::from_utf(source);
        std::string p  = arcirk::from_utf(key);
        std::vector<char> source_(s.c_str(), s.c_str() + s.size() + 1);
        std::vector<char> key_(p.c_str(), p.c_str() + p.size() + 1);
        void* text = std::data(source_);
        void* pass = std::data(key_);
        _crypt(text, ARR_SIZE(source.c_str()), pass, ARR_SIZE(key.c_str()));
        std::string result(arcirk::to_utf((char*)text));
        return result;
#else
        std::vector<char> source_(source.c_str(), source.c_str() + source.size() + 1);
        std::vector<char> key_(key.c_str(), key.c_str() + key.size() + 1);
        void* text = std::data(source_);
        void* pass = std::data(key_);
        _crypt(text, ARR_SIZE(source.c_str()), pass, ARR_SIZE(key.c_str()));
        std::string result((char*)text);
        return result;
#endif

    }

    static inline nlohmann::json json_keys(const nlohmann::json& object){
        if(!object.is_object())
            return nlohmann::json::array();
        else{
            auto result = nlohmann::json::array();
            auto items = object.items();
            for (auto itr = items.begin(); itr != items.end(); ++itr) {
                result.push_back(itr.key());
            }
            return result;
        }
    }
}

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::cryptography), cert_info,
    (std::string, serial)
    (std::string, issuer)
    (std::string, subject)
    (std::string, not_valid_before)
    (std::string, not_valid_after)
    (ByteArray, data)
    (std::string, sha1)
    (std::string, suffix)
    (std::string, cache)
    (std::string, private_key)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::cryptography), cont_info,
    (ByteArray, header_key)
    (ByteArray, masks_key)
    (ByteArray, masks2_key)
    (ByteArray, name_key)
    (ByteArray, primary_key)
    (ByteArray, primary2_key)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::cryptography), win_user_info,
    (std::string, user)
    (std::string, sid)
)

namespace arcirk::command_line {
    enum CmdCommand{
        echoSystem,
        echoUserName,
        wmicGetSID,
        echoGetEncoding,
        csptestGetConteiners,
        csptestContainerCopy,
        csptestContainerFnfo,
        csptestContainerDelete,
        csptestGetCertificates,
        certutilGetCertificateInfo,
        certmgrInstallCert,
        certmgrExportlCert,
        certmgrDeletelCert,
        certmgrGetCertificateInfo,
        cryptcpCopycert,
        mstscAddUserToConnect,
        mstscEditFile,
        quserList,
        mstscRunAsAdmin,
        cmdCD,
        cmdEXIT,
        COMMAND_INVALID=-1,
    };
}
namespace arcirk::cryptography{

    enum TypeOfStorgare{
        storgareTypeRegistry,
        storgareTypeLocalVolume,
        storgareTypeDatabase,
        storgareTypeRemoteBase,
        storgareTypeUnknown = -1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TypeOfStorgare,{
           {storgareTypeRegistry, REGISTRY},
           {storgareTypeLocalVolume, FAT12},
           {storgareTypeDatabase, DATABASE},
           {storgareTypeRemoteBase, REMOTEBASE},
    })

    inline TypeOfStorgare type_storgare(const std::string& source){
        if(source.empty())
            return storgareTypeUnknown;
        else{
            if(index_of(source, FAT12) != -1 || index_of(source, HDIMAGE) != -1)
                return storgareTypeLocalVolume;
            else if(index_of(source, REGISTRY) != -1)
                return storgareTypeRegistry;
            else if(index_of(source, DATABASE) != -1)
                return storgareTypeDatabase;
            else if(index_of(source, REMOTEBASE) != -1)
                return storgareTypeDatabase;
            else{
                return storgareTypeUnknown;
            }
        }
    }

}

#endif // SHARED_STRUCT_HPP
