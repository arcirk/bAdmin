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

#define ARCIRK_VERSION "1.1.0"
#define CONF_FILENAME "b_admin_conf.json"

typedef unsigned char BYTE;
typedef std::vector<BYTE> ByteArray;

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

        fseek(fp, 0, SEEK_END);
        size_t flen= ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::vector<unsigned char> v (flen);

        fread(&v[0], 1, flen, fp);

        fclose(fp);

        result = v;
    }
}

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
        SyncUpdateDataOnTheServer,
        WebDavServiceConfiguration,
        SyncUpdateBarcode,
        GetInformationAboutFile,
        CheckForUpdates,
        UploadFile,
        GetDatabaseTables,
        ProfileDirFileList,
        DownloadFile,
        FileToDatabase,
        ProfileDeleteFile,
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
         {GetInformationAboutFile, "GetInformationAboutFile"},
         {CheckForUpdates, "CheckForUpdates"},
         {UploadFile, "UploadFile"},
         {GetDatabaseTables, "GetDatabaseTables"},
         {ProfileDirFileList, "ProfileDirFileList"},
         {DownloadFile, "DownloadFile"},
         {FileToDatabase, "FileToDatabase"},
         {ProfileDeleteFile, "ProfileDeleteFile"}    ,
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
    });

    enum application_names{
        PriceChecker,
        ServerManager,
        APP_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(application_names, {
        {APP_INVALID, nullptr} ,
        {PriceChecker, "PriceChecker"} ,
        {ServerManager, "ServerManager"} ,
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

static inline std::string crypt(const std::string &source, const std::string& key) {

    void * text = (void *) source.c_str();
    void * pass = (void *) key.c_str();
    _crypt(text, ARR_SIZE(source.c_str()), pass, ARR_SIZE(key.c_str()));

    std::string result((char*)text);


    return result;
}

#endif // SHARED_STRUCT_HPP
