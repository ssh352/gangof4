﻿#ifndef FEMAS_TRADE_INTERFACE_H_
#define FEMAS_TRADE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <set>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

#include "USTPFtdcTraderApi.h"
#include "config_data.h"
#include "trade_data_type.h"
#include "my_tunnel_lib.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "femas_trade_context.h"
#include "field_convert.h"

struct OriginalReqInfo;

class MyFemasTradeSpi: public CUstpFtdcTraderSpi
{
public:
    MyFemasTradeSpi(const TunnelConfigData &cfg);
    virtual ~MyFemasTradeSpi(void);

    void SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler);
    void SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler);
    void SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler);
    void SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler);

    void SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
    {
        QryPosReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
    {
        QryOrderDetailReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
    {
        QryTradeDetailReturnHandler_ = handler;
    }
    bool ParseConfig();

    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();

    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误原因
    ///        0x1001 网络读失败
    ///        0x1002 网络写失败
    ///        0x2001 接收心跳超时
    ///        0x2002 发送心跳失败
    ///        0x2003 收到错误报文
    virtual void OnFrontDisconnected(int nReason);

    ///心跳超时警告。当长时间未收到报文时，该方法被调用。
    ///@param nTimeLapse 距离上次接收报文的时间
    virtual void OnHeartBeatWarning(int nTimeLapse);

    ///报文回调开始通知。当API收到一个报文后，首先调用本方法，然后是各数据域的回调，最后是报文回调结束通知。
    ///@param nTopicID 主题代码（如私有流、公共流、行情流等）
    ///@param nSequenceNo 报文序号
    virtual void OnPackageStart(int nTopicID, int nSequenceNo);

    ///报文回调结束通知。当API收到一个报文后，首先调用报文回调开始通知，然后是各数据域的回调，最后调用本方法。
    ///@param nTopicID 主题代码（如私有流、公共流、行情流等）
    ///@param nSequenceNo 报文序号
    virtual void OnPackageEnd(int nTopicID, int nSequenceNo);

    ///错误应答
    virtual void OnRspError(CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///风控前置系统用户登录应答
    virtual void OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///用户退出应答
    virtual void OnRspUserLogout(CUstpFtdcRspUserLogoutField *pRspUserLogout, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///用户密码修改应答
    virtual void OnRspUserPasswordUpdate(CUstpFtdcUserPasswordUpdateField *pUserPasswordUpdate, CUstpFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast)
    {
    }

    ///报单录入应答
    virtual void OnRspOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///报单操作应答
    virtual void OnRspOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///数据流回退通知
    virtual void OnRtnFlowMessageCancel(CUstpFtdcFlowMessageCancelField *pFlowMessageCancel)
    {
    }

    ///成交回报
    virtual void OnRtnTrade(CUstpFtdcTradeField *pTrade);

    ///报单回报
    virtual void OnRtnOrder(CUstpFtdcOrderField *pOrder);

    ///报单录入错误回报
    virtual void OnErrRtnOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo);

    ///报单操作错误回报
    virtual void OnErrRtnOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo);

    ///合约交易状态通知
    virtual void OnRtnInstrumentStatus(CUstpFtdcInstrumentStatusField *pInstrumentStatus)
    {
    }

    ///报单查询应答
    virtual void OnRspQryOrder(CUstpFtdcOrderField *pOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///成交单查询应答
    virtual void OnRspQryTrade(CUstpFtdcTradeField *pTrade, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///可用投资者账户查询应答
    virtual void OnRspQryUserInvestor(CUstpFtdcRspUserInvestorField *pRspUserInvestor, CUstpFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast)
    {
    }

    ///交易编码查询应答
    virtual void OnRspQryTradingCode(CUstpFtdcRspTradingCodeField *pRspTradingCode, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
    {
    }

    ///投资者资金账户查询应答
    virtual void OnRspQryInvestorAccount(CUstpFtdcRspInvestorAccountField *pRspInvestorAccount, CUstpFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast)
    {
    }

    ///合约查询应答
    virtual void OnRspQryInstrument(CUstpFtdcRspInstrumentField *pRspInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
    {
    }

    ///交易所查询应答
    virtual void OnRspQryExchange(CUstpFtdcRspExchangeField *pRspExchange, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
    {
    }

    ///投资者持仓查询应答
    virtual void OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pRspInvestorPosition, CUstpFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    ///订阅主题应答
    virtual void OnRspSubscribeTopic(CUstpFtdcDisseminationField *pDissemination, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
    {
    }

    ///合规参数查询应答
    virtual void OnRspQryComplianceParam(CUstpFtdcRspComplianceParamField *pRspComplianceParam, CUstpFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast)
    {
    }

    ///主题查询应答
    virtual void OnRspQryTopic(CUstpFtdcDisseminationField *pDissemination, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
    {
    }

    ///投资者手续费率查询应答
    virtual void OnRspQryInvestorFee(CUstpFtdcInvestorFeeField *pInvestorFee, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
    {
    }

    ///投资者保证金率查询应答
    virtual void OnRspQryInvestorMargin(CUstpFtdcInvestorMarginField *pInvestorMargin, CUstpFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast)
    {
    }

public:
    // 下发指令接口
    int ReqOrderInsert(CUstpFtdcInputOrderField *pInputOrder, int nRequestID)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderInsert(pInputOrder, nRequestID);
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqOrderInsert.");
        }

        return ret;
    }

    //报单操作请求
    int ReqOrderAction(CUstpFtdcOrderActionField *pInputOrderAction, int nRequestID)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderAction(pInputOrderAction, nRequestID);
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqOrderInsert.");
        }

        return ret;
    }

    int QryPosition(CUstpFtdcQryInvestorPositionField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryPosition when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = api_->ReqQryInvestorPosition(p, request_id);
        TNL_LOG_INFO("ReqQryInvestorPosition - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryOrderDetail(CUstpFtdcQryOrderField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = api_->ReqQryOrder(p, request_id);
        TNL_LOG_INFO("ReqQryOrder - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryTradeDetail(CUstpFtdcQryTradeField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = api_->ReqQryTrade(p, request_id);
        TNL_LOG_INFO("ReqQryTrade - request_id:%d, return:%d", request_id, ret);

        return ret;
    }

    int Front_id() const
    {
        return front_id_;
    }
    int Session_id() const
    {
        return session_id_;
    }

private:
    bool TunnelIsReady()
    {
        return logoned_ && HaveFinishQueryOrders();
    }
    bool HaveFinishQueryOrders()
    {
        return have_handled_unterminated_orders_ && finish_qryorder_result_stats_;
    }


    void OnRtnOrderFak(CUstpFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref);
    void OnRtnOrderNormal(CUstpFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref);

    void HandleFillupRsp(long entrust_no, SerialNoType serial_no);
    void HandleFillupRtn(long entrust_no, char order_status, const OriginalReqInfo *p);

public:
    FEMASTradeContext femas_trade_context_;
    std::mutex client_sync;

    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    CUstpFtdcTraderApi *api_;
    int front_id_;
    int session_id_;
    OrderRefDataType max_order_ref_;

    Tunnel_Info tunnel_info_;
    std::string user_;
    std::string pswd_;
    std::string quote_addr_;
    std::string exchange_code_;

    std::function<void(const T_OrderRespond *)> OrderRespond_call_back_handler_;
    std::function<void(const T_CancelRespond *)> CancelRespond_call_back_handler_;
    std::function<void(const T_OrderReturn *)> OrderReturn_call_back_handler_;
    std::function<void(const T_TradeReturn *)> TradeReturn_call_back_handler_;

    std::function<void(const T_PositionReturn *)> QryPosReturnHandler_;
    std::function<void(const T_OrderDetailReturn *)> QryOrderDetailReturnHandler_;
    std::function<void(const T_TradeDetailReturn *)> QryTradeDetailReturnHandler_;

    // 配置数据对象
    TunnelConfigData cfg_;
    volatile bool connected_;
    std::atomic_bool logoned_;

    // query position control variables
    std::vector<PositionDetail> pos_buffer_;

    // query order detail control variables
    std::vector<OrderDetail> od_buffer_;

    // query trade detail control variables
    std::vector<TradeDetail> td_buffer_;

    // 查询报单
    void QueryAndHandleOrders();
    void ReportErrorState(int api_error_no, const std::string &error_msg);

    // variables and functions for cancel all unterminated orders automatically
    std::atomic_bool have_handled_unterminated_orders_;
    std::vector<CUstpFtdcOrderField> unterminated_orders_;
    std::mutex cancel_sync_;
    std::condition_variable qry_order_finish_cond_;
    CUstpFtdcOrderActionField CreatCancelParam(const CUstpFtdcOrderField & order_field);
    bool IsOrderTerminate(const CUstpFtdcOrderField & order_field);

    std::mutex qry_order_result_stats_sync_;
    std::atomic_bool finish_qryorder_result_stats_;
    std::map<std::string, int> cancel_times_of_contract;
    std::map<std::string, int> open_volume_of_product;

    volatile bool in_init_state_; // clear after login
};

#endif //