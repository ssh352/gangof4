﻿/**
 * 版权所有(C)2012-2016, 大连飞创信息技术有限公司
 * 文件名称：DFITCMdExtApi.h
 * 文件说明：定义XSpeed行情API接口
 * 当前版本：1.0.13
 * 作者：XSpeed项目组
 * 发布日期：2013年8月15日
 */

#ifndef DLMDEXT_API_H_
#define DLMDEXT_API_H_


#include "DFITCApiStruct.h"


#ifdef WIN32
    #ifdef DFITCAPI_EXPORTS
        #define MDEXTAPI_API __declspec(dllexport)
    #else
        #define MDEXTAPI_API __declspec(dllimport)
    #endif//DFITCAPI_EXPORTS
#else
    #define MDEXTAPI_API 
#endif//WIN32

namespace DFITCXSPEEDMDEXTAPI
{
    class DFITCMdExtSpi
    {
    public:
        DFITCMdExtSpi(){}

        /** 
         * 网络连接正常响应
         */
        virtual void OnFrontConnected(){};

        /**
         * 网络连接不正常响应
         */
        virtual void OnFrontDisconnected(int nReason){};

        /**
         * 登陆请求响应:当用户发出登录请求后，前置机返回响应时此方法会被调用，通知用户登录是否成功。
         * @param pRspUserLogin:用户登录信息结构地址。
         * @param pRspInfo:若请求失败，返回错误信息地址，该结构含有错误信息。
         */
        virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pRspUserLogin, struct DFITCErrorRtnField * pRspInfo) {}

        /**
         * 登出请求响应:当用户发出退出请求后，前置机返回响应此方法会被调用，通知用户退出状态。
         * @param pRspUsrLogout:返回用户退出信息结构地址。
         * @param pRspInfo:若请求失败，返回错误信息地址。
         */
        virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pRspUsrLogout, struct DFITCErrorRtnField * pRspInfo) {}

        /**
		 * 错误应答
		 * @param pRspInfo:错误信息地址。
		 */
        virtual void OnRspError(struct DFITCErrorRtnField *pRspInfo) {}

        /**
         * 行情订阅应答:当用户发出行情订阅该方法会被调用。
         * @param pSpecificInstrument:指向合约响应结构，该结构包含合约的相关信息。
         * @param pRspInfo:错误信息，如果发生错误，该结构含有错误信息。
         */
        virtual void OnRspSubMarketDataEX(struct DFITCSpecificInstrumentFieldEX * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo) {}

        /**
         * 取消订阅行情应答:当用户发出退订请求后该方法会被调用。
         * @param pSpecificInstrument:指向合约响应结构，该结构包含合约的相关信息。
         * @param pRspInfo:错误信息，如果发生错误，该结构含有错误信息。
         */
        virtual void OnRspUnSubMarketDataEX(struct DFITCSpecificInstrumentFieldEX * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo) {}

        /**
         * 行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
         * @param pMarketDataField:指向行情信息结构的指针，结构体中包含具体的行情信息。
         */
        virtual void OnOptionComputationData(struct DFITCMarketDataFieldEx * pMarketDataField) {}

        /**
         * 交易日确认响应:用于接收交易日信息。
         * @param pTradingDayRtnData: 返回交易日请求确认响应结构的地址。
         */
        virtual void OnRspTradingDay(struct DFITCTradingDayRtnField * pTradingDayRtnData){};
    };//end of DFITCMdSpi

    class MDEXTAPI_API DFITCMdExtApi
    {
    public:
        /**
         * 创建行情API实例
         * @return 创建出的UserApi
         */
        static DFITCMdExtApi * CreateDFITCMdExtApi();

        /**
         * 进行一系列初始化工作:注册回调函数接口,连接行情前置。
         * @param pszSvrAddr:行情前置网络地址。
         *                  网络地址的格式为:"protocol://ipaddress:port",如"tcp://127.0.0.1:10915"
         *                  其中protocol的值为tcp或者udp,表示接收行情的方式;如果是udp接收行情数据,udp的端口将由API自行确定。
         *                  ipaddress表示行情前置的IP,port表示行情前置的端口
         * @param pSpi:类DFITCMdSpi对象实例
         * @return 0 - 成功; -1 - 失败。
         */
        virtual int Init(char * pszSvrAddr, DFITCMdExtSpi * pSpi) = 0;

        /**
         * 删除接口对象本身，不再使用本接口对象时,调用该函数删除接口对象。
         */
        virtual void Release() = 0;

        /**
         * 订阅行情:该方法发出订阅某个或者某些合约行情请求。

         * @param ppInstrumentID[]:指针数组，每个指针指向一个合约。 （当输入“m”时，订阅所有合约）
         * @param nCount:合约个数
         * @return 0 - 请求发送成功; -1 - 请求发送失败。
         */
        virtual int SubscribeMarketData(char * ppInstrumentID[], int nCount, int nRequestID) = 0;

        /**
         * 退订行情:该方法发出退订某个/某些合约行情请求。
         * @param ppInstrumentID[]:指针数组，每个指针指向一个合约。 （当输入“m”时，退订所有合约）
         * @param nCount:合约个数
         * @return 0 - 请求发送成功; -1 - 请求发送失败
         */
        virtual int UnSubscribeMarketData(char * ppInstrumentID[], int nCount, int nRequestID) = 0;

        /**
         * 用户发出登陆请求
         * @param pReqUserLoginField:指向用户登录请求结构的地址。
         * @return 0 - 请求发送成功; -1 - 请求发送失败; -2 - 检测异常。
         */ 
        virtual int ReqUserLogin(struct DFITCUserLoginField * pReqUserLoginField) = 0;

        /**
         * 用户发出登出请求
         * @param pReqUserLogoutField:指向用户登录请出结构的地址。
         * @return 0 - 请求发送成功; -1 - 请求发送失败; -2 - 检测异常。
         */ 
        virtual int ReqUserLogout(struct DFITCUserLogoutField * pReqUserLogoutField) = 0;

        /**
         * 交易日查询请求
         * @return 0 - 请求发送成功; -1 - 请求发送失败。
         */
        virtual int ReqTradingDay(struct DFITCTradingDayField * pTradingDay) = 0;

    protected:
        virtual ~DFITCMdExtApi() = 0;

    };//end of MDEXTAPI_API
} //end of namespace

#endif//DLMDEXT_API_H_
