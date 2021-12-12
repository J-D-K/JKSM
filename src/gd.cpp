#include <stdio.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <string>
#include <vector>

#include "gd.h"
#include "fs.h"
#include "util.h"
#include "curlfuncs.h"
#include "sys.h"

/*
Google Drive code for JKSV.
Modified 3DS version
Still WIP
*/

#define DRIVE_UPLOAD_BUFFER_SIZE 0x8000

#define tokenURL "https://oauth2.googleapis.com/token"
#define tokenCheckURL "https://oauth2.googleapis.com/tokeninfo"
#define driveURL "https://www.googleapis.com/drive/v3/files"
#define driveUploadURL "https://www.googleapis.com/upload/drive/v3/files"
#define userAgent "JKSM"

drive::gd::gd(const std::string &_clientID, const std::string& _secretID, const std::string& _authCode, const std::string& _rToken)
{
    clientID = _clientID;
    secretID = _secretID;
    rToken = _rToken;

    if(!_authCode.empty())
        exhangeAuthCode(_authCode);
    else if(!rToken.empty())
        refreshToken();
}

void drive::gd::exhangeAuthCode(const std::string& _authCode)
{
    // Header
    curl_slist *postHeader = NULL;
    postHeader = curl_slist_append(postHeader, HEADER_CONTENT_TYPE_APP_JSON);

    // Post json
    json_object *post = json_object_new_object();
    json_object *clientIDString = json_object_new_string(clientID.c_str());
    json_object *secretIDString = json_object_new_string(secretID.c_str());
    json_object *authCodeString = json_object_new_string(_authCode.c_str());
    json_object *redirectUriString = json_object_new_string("urn:ietf:wg:oauth:2.0:oob");
    json_object *grantTypeString = json_object_new_string("authorization_code");
    json_object_object_add(post, "client_id", clientIDString);
    json_object_object_add(post, "client_secret", secretIDString);
    json_object_object_add(post, "code", authCodeString);
    json_object_object_add(post, "redirect_uri", redirectUriString);
    json_object_object_add(post, "grant_type", grantTypeString);

    // Curl Request
    std::string *jsonResp = new std::string;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, postHeader);
    curl_easy_setopt(curl, CURLOPT_URL, tokenURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, jsonResp);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_get_string(post));
    
    int error = curl_easy_perform(curl);

    json_object *respParse = json_tokener_parse(jsonResp->c_str());
    if (error == CURLE_OK)
    {
        json_object *accessToken = json_object_object_get(respParse, "access_token");
        json_object *refreshToken = json_object_object_get(respParse, "refresh_token");

        if(accessToken && refreshToken)
        {
            token = json_object_get_string(accessToken);
            rToken = json_object_get_string(refreshToken);
        }
    }

    delete jsonResp;
    json_object_put(post);
    json_object_put(respParse);
    curl_slist_free_all(postHeader);
    curl_easy_cleanup(curl);
}

void drive::gd::refreshToken()
{
    // Header
    curl_slist *header = NULL;
    header = curl_slist_append(header, HEADER_CONTENT_TYPE_APP_JSON);

    // Post Json
    json_object *post = json_object_new_object();
    json_object *clientIDString = json_object_new_string(clientID.c_str());
    json_object *secretIDString = json_object_new_string(secretID.c_str());
    json_object *refreshTokenString = json_object_new_string(rToken.c_str());
    json_object *grantTypeString = json_object_new_string("refresh_token");
    json_object_object_add(post, "client_id", clientIDString);
    json_object_object_add(post, "client_secret", secretIDString);
    json_object_object_add(post, "refresh_token", refreshTokenString);
    json_object_object_add(post, "grant_type", grantTypeString);

    // Curl
    std::string *jsonResp = new std::string;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_URL, tokenURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, jsonResp);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_get_string(post));
    int error = curl_easy_perform(curl);

    json_object *parse = json_tokener_parse(jsonResp->c_str());
    if (error == CURLE_OK)
    {
        json_object *accessToken, *error;
        json_object_object_get_ex(parse, "access_token", &accessToken);
        json_object_object_get_ex(parse, "error", &error);

        if(accessToken)
            token = json_object_get_string(accessToken);
    }

    delete jsonResp;
    json_object_put(post);
    json_object_put(parse);
    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
}

bool drive::gd::tokenIsValid()
{
    bool ret = false;

    std::string url = tokenCheckURL;
    url.append("?access_token=" + token);

    CURL *curl = curl_easy_init();
    std::string *jsonResp = new std::string;
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, jsonResp);

    int error = curl_easy_perform(curl);
    json_object *parse = json_tokener_parse(jsonResp->c_str());
    if (error == CURLE_OK)
    {
        json_object *checkError;
        json_object_object_get_ex(parse, "error", &checkError);
        if(!checkError)
            ret = true;
    }

    delete jsonResp;
    json_object_put(parse);
    curl_easy_cleanup(curl);
    return ret;
}

void drive::gd::loadDriveList()
{
    if(!tokenIsValid())
        refreshToken();

    // Request url with specific fields needed.
    std::string url = driveURL;
    url.append("?fields=files(name,id,mimeType,size,parents)&q=trashed=false\%20and\%20\%27me\%27\%20in\%20owners");

    // Headers needed
    curl_slist *postHeaders = NULL;
    postHeaders = curl_slist_append(postHeaders, std::string(HEADER_AUTHORIZATION + token).c_str());

    // Curl request
    std::string *jsonResp = new std::string;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, postHeaders);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, jsonResp);

    int error = curl_easy_perform(curl);

    json_object *parse = json_tokener_parse(jsonResp->c_str());
    if (error == CURLE_OK)
    {
        driveList.clear();
        json_object *fileArray;
        json_object_object_get_ex(parse, "files", &fileArray);
        if(fileArray)
        {
            size_t count = json_object_array_length(fileArray);
            driveList.reserve(count);
            for (unsigned i = 0; i < count; i++)
            {
                json_object *idString, *nameString, *mimeTypeString, *size, *parentArray;
                json_object *curFile = json_object_array_get_idx(fileArray, i);
                json_object_object_get_ex(curFile, "id", &idString);
                json_object_object_get_ex(curFile, "name", &nameString);
                json_object_object_get_ex(curFile, "mimeType", &mimeTypeString);
                json_object_object_get_ex(curFile, "size", &size);
                json_object_object_get_ex(curFile, "parents", &parentArray);

                drive::gdItem newItem;
                newItem.name = json_object_get_string(nameString);
                newItem.id = json_object_get_string(idString);
                newItem.size = json_object_get_int(size);
                if(strcmp(json_object_get_string(mimeTypeString), MIMETYPE_FOLDER) == 0)
                    newItem.isDir = true;

                if (parentArray)
                {
                    size_t parentCount = json_object_array_length(parentArray);
                    for (unsigned j = 0; j < parentCount; j++)
                    {
                        json_object *parent = json_object_array_get_idx(parentArray, j);
                        newItem.parent = json_object_get_string(parent);
                    }
                }
                driveList.push_back(newItem);
            }
        }
    }
    delete jsonResp;
    json_object_put(parse);
    curl_slist_free_all(postHeaders);
    curl_easy_cleanup(curl);
}

void drive::gd::getListWithParent(const std::string& _parent, std::vector<drive::gdItem *>& _out)
{
    _out.clear();
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(driveList[i].parent == _parent)
            _out.push_back(&driveList[i]);
    }
}

void drive::gd::debugWriteList()
{
    fs::fsfile list(fs::getSDMCArch(), "/JKSV/drive_list.txt", FS_OPEN_CREATE | FS_OPEN_WRITE);
    for(auto& di : driveList)
    {
        list.writef("%s\n\t%s\n", di.name.c_str(), di.id.c_str());
        if(!di.parent.empty())
            list.writef("\t%s\n", di.parent.c_str());
    }
}

bool drive::gd::createDir(const std::string& _dirName, const std::string& _parent)
{
    if(!tokenIsValid())
        refreshToken();

    bool ret = true;

    // Headers to use
    curl_slist *postHeaders = NULL;
    postHeaders = curl_slist_append(postHeaders, std::string(HEADER_AUTHORIZATION + token).c_str());
    postHeaders = curl_slist_append(postHeaders, HEADER_CONTENT_TYPE_APP_JSON);

    // JSON To Post
    json_object *post = json_object_new_object();
    json_object *nameString = json_object_new_string(_dirName.c_str());
    json_object *mimeTypeString = json_object_new_string(MIMETYPE_FOLDER);
    json_object_object_add(post, "name", nameString);
    json_object_object_add(post, "mimeType", mimeTypeString);
    if (!_parent.empty())
    {
        json_object *parentsArray = json_object_new_array();
        json_object *parentString = json_object_new_string(_parent.c_str());
        json_object_array_add(parentsArray, parentString);
        json_object_object_add(post, "parents", parentsArray);
    }

    // Curl Request
    std::string *jsonResp = new std::string;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, postHeaders);
    curl_easy_setopt(curl, CURLOPT_URL, driveURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, jsonResp);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_get_string(post));
    int error = curl_easy_perform(curl);
    
    json_object *respParse = json_tokener_parse(jsonResp->c_str()), *checkError;
    json_object_object_get_ex(respParse, "error", &checkError);
    if (error == CURLE_OK && !checkError)
    {
        //Append it to list
        json_object *id;
        json_object_object_get_ex(respParse, "id", &id);

        drive::gdItem newDir;
        newDir.name = _dirName;
        newDir.id = json_object_get_string(id);
        newDir.isDir = true;
        newDir.size = 0;
        newDir.parent = _parent;
        driveList.push_back(newDir);
    }
    else
        ret = false;

    delete jsonResp;
    json_object_put(post);
    json_object_put(respParse);
    curl_slist_free_all(postHeaders);
    curl_easy_cleanup(curl);
    return ret;
}

bool drive::gd::dirExists(const std::string& _dirName)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(driveList[i].isDir && driveList[i].name == _dirName)
            return true;
    }
    return false;
}

bool drive::gd::dirExists(const std::string& _dirName, const std::string& _parent)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(driveList[i].isDir && driveList[i].name == _dirName && driveList[i].parent == _parent)
            return true;
    }
    return false;
}

bool drive::gd::fileExists(const std::string& _filename)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(!driveList[i].isDir && driveList[i].name == _filename)
            return true;
    }
    return false;
}

bool drive::gd::fileExists(const std::string& _filename, const std::string& _parent)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(!driveList[i].isDir && driveList[i].name == _filename)
            return true;
    }
    return false;
}

void drive::gd::uploadFile(const std::string& _filename, const std::string& _parent, FILE *_upload)
{
    if(!tokenIsValid())
        refreshToken();

    std::string url = driveUploadURL;
    url.append("?uploadType=resumable");

    // Headers
    curl_slist *postHeaders = NULL;
    postHeaders = curl_slist_append(postHeaders, std::string(HEADER_AUTHORIZATION + token).c_str());
    postHeaders = curl_slist_append(postHeaders, HEADER_CONTENT_TYPE_APP_JSON);

    // Post JSON
    json_object *post = json_object_new_object();
    json_object *nameString = json_object_new_string(_filename.c_str());
    json_object_object_add(post, "name", nameString);
    if (!_parent.empty())
    {
        json_object *parentArray = json_object_new_array();
        json_object *parentString = json_object_new_string(_parent.c_str());
        json_object_array_add(parentArray, parentString);
        json_object_object_add(post, "parents", parentArray);
    }

    // Curl upload request
    std::string *jsonResp = new std::string;
    std::vector<std::string> *headers = new std::vector<std::string>;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, postHeaders);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlFuncs::writeHeaders);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_get_string(post));
    
    int error = curl_easy_perform(curl);
    std::string location = curlFuncs::getHeader("Location", headers);
    if (error == CURLE_OK && location != HEADER_ERROR)
    {
        CURL *curlUp = curl_easy_init();
        curl_easy_setopt(curlUp, CURLOPT_PUT, 1);
        curl_easy_setopt(curlUp, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curlUp, CURLOPT_URL, location.c_str());
        curl_easy_setopt(curlUp, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataString);
        curl_easy_setopt(curlUp, CURLOPT_WRITEDATA, jsonResp);
        curl_easy_setopt(curlUp, CURLOPT_READFUNCTION, curlFuncs::readDataFile);
        curl_easy_setopt(curlUp, CURLOPT_READDATA, _upload);
        curl_easy_setopt(curlUp, CURLOPT_UPLOAD_BUFFERSIZE, DRIVE_UPLOAD_BUFFER_SIZE);
        curl_easy_setopt(curlUp, CURLOPT_UPLOAD, 1);
        curl_easy_perform(curlUp);
        curl_easy_cleanup(curlUp);

        json_object *parse = json_tokener_parse(jsonResp->c_str()), *id, *name, *mimeType;
        json_object_object_get_ex(parse, "id", &id);
        json_object_object_get_ex(parse, "name", &name);
        json_object_object_get_ex(parse, "mimeType", &mimeType);

        if(name && id && mimeType)
        {
            drive::gdItem uploadData;
            uploadData.id = json_object_get_string(id);
            uploadData.name = json_object_get_string(name);
            uploadData.isDir = false;
            uploadData.parent = _parent;
            driveList.push_back(uploadData);
        }
        json_object_put(parse);
    }

    delete jsonResp;
    delete headers;
    json_object_put(post);
    curl_slist_free_all(postHeaders);
}

void drive::gd::updateFile(const std::string& _fileID, FILE *_upload)
{
    if(!tokenIsValid())
        refreshToken();

    //URL
    std::string url = driveUploadURL;
    url.append("/" + _fileID);
    url.append("?uploadType=resumable");

    //Header
    curl_slist *patchHeader = NULL;
    patchHeader = curl_slist_append(patchHeader, std::string(HEADER_AUTHORIZATION + token).c_str());

    //Curl
    std::string *jsonResp = new std::string;
    std::vector<std::string> *headers = new std::vector<std::string>;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, patchHeader);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataString);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlFuncs::writeHeaders);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, jsonResp);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, headers);
    
    int error = curl_easy_perform(curl);
    std::string location = curlFuncs::getHeader("Location", headers);
    if(error == CURLE_OK && location != HEADER_ERROR)
    {
        CURL *curlPatch = curl_easy_init();
        curl_easy_setopt(curlPatch, CURLOPT_PUT, 1);
        curl_easy_setopt(curlPatch, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curlPatch, CURLOPT_URL, location.c_str());
        curl_easy_setopt(curlPatch, CURLOPT_READFUNCTION, curlFuncs::readDataFile);
        curl_easy_setopt(curlPatch, CURLOPT_READDATA, _upload);
        curl_easy_setopt(curlPatch, CURLOPT_UPLOAD_BUFFERSIZE, DRIVE_UPLOAD_BUFFER_SIZE);
        curl_easy_setopt(curlPatch, CURLOPT_UPLOAD, 1);
        curl_easy_perform(curlPatch);
        curl_easy_cleanup(curlPatch);
    }

    delete jsonResp;
    delete headers;
    curl_slist_free_all(patchHeader);
    curl_easy_cleanup(curl);
}

void drive::gd::downloadFile(const std::string& _fileID, FILE *_download)
{
    if(!tokenIsValid())
        refreshToken();

    //URL
    std::string url = driveURL;
    url.append("/" + _fileID);
    url.append("?alt=media");

    //Headers
    curl_slist *getHeaders = NULL;
    getHeaders = curl_slist_append(getHeaders, std::string(HEADER_AUTHORIZATION + token).c_str());

    //Curl
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, getHeaders);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 0x8000);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlFuncs::writeDataFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, _download);
    int error = curl_easy_perform(curl);
    curl_slist_free_all(getHeaders);
    curl_easy_cleanup(curl);
}

void drive::gd::deleteFile(const std::string& _fileID)
{
    if(!tokenIsValid())
        refreshToken();

    //URL
    std::string url = driveURL;
    url.append("/" + _fileID);

    //Header
    curl_slist *delHeaders = NULL;
    delHeaders = curl_slist_append(delHeaders, std::string(HEADER_AUTHORIZATION + token).c_str());

    //Curl
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, delHeaders);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    int error = curl_easy_perform(curl);

    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(driveList[i].id == _fileID)
        {
            driveList.erase(driveList.begin() + i);
            break;
        }
    }

    curl_slist_free_all(delHeaders);
    curl_easy_cleanup(curl);
}

std::string drive::gd::getFolderID(const std::string& _name)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(driveList[i].name == _name && driveList[i].isDir)
            return driveList[i].id;
    }
    return "";
}

std::string drive::gd::getFolderID(const std::string& _name, const std::string& _parent)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(driveList[i].isDir && driveList[i].name == _name && driveList[i].parent == _parent)
            return driveList[i].id;
    }
    return "";
}

std::string drive::gd::getFileID(const std::string& _name)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(driveList[i].name == _name && driveList[i].isDir == false)
            return driveList[i].id;
    }
    return "";
}

std::string drive::gd::getFileID(const std::string& _name, const std::string& _parent)
{
    for(unsigned i = 0; i < driveList.size(); i++)
    {
        if(!driveList[i].isDir && driveList[i].name == _name && driveList[i].parent == _parent)
            return driveList[i].id;
    }
    return "";
}