#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "hbfilter.h"
#include "util.h"
#include "ui.h"

std::vector<u32> filterID;

bool downloadFilter()
{
    u32 wifi;
    ACU_GetWifiStatus(&wifi);
    if(wifi == 0)
        return false;

    httpcContext filter;

    Result res = httpcOpenContext(&filter, HTTPC_METHOD_GET, "https://raw.githubusercontent.com/J-D-K/JKSM/master/filter.txt", 1);
    if(res)
    {
        showError("Open context.", (unsigned)res);
        return false;
    }

    res = httpcSetSSLOpt(&filter, SSLCOPT_DisableVerify);
    if(res)
    {
        showError("Set SSL Opt.", (u32)res);
    }

    res = httpcBeginRequest(&filter);
    if(res)
    {
        showError("Begin request.", (u32)res);
        return false;
    }

    u32 code;
    res = httpcGetResponseStatusCode(&filter, &code);
    if(res || code != 200)
    {
        showMessage("Not found?", "wut");
        return false;
    }

    u32 fSize;
    res = httpcGetDownloadSizeState(&filter, NULL, &fSize);
    if(res)
    {
        showError("Download size.", (u32)res);
        return false;
    }

    u8 *buff = new u8[fSize];
    res = httpcDownloadData(&filter, buff, fSize, NULL);
    if(res)
    {
        showError("Download data.", (u32)res);
        delete[] buff;
        return false;
    }

    FILE *filterTxt = fopen("filter.txt", "wb");
    fwrite(buff, 1, fSize, filterTxt);
    fclose(filterTxt);

    delete[] buff;

    showMessage("Filter downloaded.", "Info");

    return true;
}

void loadFilterList()
{
    filterID.clear();
    if(fexists("filter.txt"))
    {
        FILE *load = fopen("filter.txt", "r");

        char id[16];

        while(fgets(id, 16, load))
        {
            if(id[0] != '#')
            {
                u32 newID = strtoul(id, NULL, 16);
                filterID.push_back(newID);
            }
        }

        fclose(load);
    }
    else
    {
        if(downloadFilter())
            loadFilterList();
    }
}

bool hbFilter(u64 id)
{
    u32 low = (u32)id;
    //Don't need code specifically for retroarch.
    //Thank you ksanislo for titledb
    for(unsigned i = 0; i < filterID.size(); i++)
    {
        if(low == filterID[i])
            return true;
    }
    return false;
}
