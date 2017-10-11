package main

import (
	"bytes"
	"database/sql"
	"fmt"
	//log2 "github.com/Sirupsen/logrus"
	JSON "github.com/bitly/go-simplejson"
	_ "github.com/go-sql-driver/mysql"
	"net"
	"os"
	"runtime"
	"strconv"
	"sync"
	"time"
)

const (
	RECV_BUF_LEN       = 10 * 1024
	MAX_BUF      int64 = 1024 * 1024
	TIMEOUT            = time.Second * 120 //超时时间 65秒
)

type Client struct {
	m_sessionID     int64         //用户唯一标识，可能是用户编号，也可能是随机数
	m_userName      string        //用户名
	m_clientType    int           //客户端类型
	m_userRole      int           //用户类型
	m_currentRoomID int           //当前进入的房间号
	m_conn          *net.TCPConn  //这个用户的TCP连接
	Allbuf          *bytes.Buffer //接收缓冲区
}

type RoomInfo struct {
	shareDoc     int    //是否在共享文档 1：是 0：不是
	shareDocUser int64  //当前操作文档的用户Id
	docIndex     int    //当前第几页
	docTotal     int    //一共多少页
	docName      string //共享文档Url
}

var (
	clientList map[int64]*Client = make(map[int64]*Client, 1000) //用户列表
	roomList   map[int]*RoomInfo = make(map[int]*RoomInfo, 1000)
	db         *sql.DB
	err        error //数据库连接
	mutex      sync.Mutex
)

//房间配置

var (
	MixAudioPort, RtpMediaPort, Version int
	MediaHost, BaseUrl                  string
)

//产生新的Client对象
func NewClient(conn *net.TCPConn) (cli *Client) {
	fmt.Printf("新的连接进来%p\n", &conn)
	//设置50秒超时
	//conn.SetReadDeadline(time.Now().Add(TIMEOUT))
	//addr := conn.RemoteAddr().String()
	//addrarray := strings.Split(addr, ":")
	cli = new(Client)
	cli.m_conn = conn
	cli.Allbuf = bytes.NewBuffer(nil)
	return
}

//处理新连接
func ServiceClient(conn *net.TCPConn) {

	control := true
	readbuf := make([]byte, RECV_BUF_LEN)

	clientAddr := conn.RemoteAddr()
	client := NewClient(conn)
	for control {
		n, err := conn.Read(readbuf)
		if err != nil {
			fmt.Println(clientAddr, "连接异常!", err)
			conn.Close()
			control = false
			OnLogout(client, nil)
			break
		}
		if n <= 0 {
			fmt.Println("客户端关闭连接")
			control = false
			break
		}
		PackageSplit(client, readbuf[:n])
	}
}

//拼包，拆包
//拆分包 /r/n->13 10
func PackageSplit(client *Client, buf []byte) {
	k := 0
	for i, byteval := range buf {
		//1: i > 0 && buf[i-1] == 13
		//2: i==0 && client.Allbuf.Len() > 0 && client.Allbuf.Bytes()[client.Allbuf.Len()-1] == 13
		if byteval == 10 {
			if i > 0 && buf[i-1] == 13 {
				//读取到包结束符号
				client.Allbuf.Write(buf[k : i-1])
				PackageHandle(client, client.Allbuf.Bytes())
				client.Allbuf.Reset()
				k = i + 1
			} else if i == 0 && client.Allbuf.Len() > 0 && client.Allbuf.Bytes()[client.Allbuf.Len()-1] == 13 {
				//读取到包结束符号
				PackageHandle(client, client.Allbuf.Bytes()[:client.Allbuf.Len()-1])
				client.Allbuf.Reset()
				k = i + 1
			}
		}

	}
	//检测是否还有半截包
	if k < len(buf) {
		client.Allbuf.Write(buf[k:])

		if int64(client.Allbuf.Len()) > MAX_BUF {
			client.m_conn.Close()
		}
		//fmt.Println("半截包内容:", string(client.Allbuf.Bytes()))
	}
}

func PackageHandle(client *Client, bytes []byte) {
	//转为JSON
	//fmt.Println("消息内容：", string(bytes))

	jsonObject, err := JSON.NewJson(bytes)
	if err != nil {
		fmt.Println("解析JSON异常：", err)
		return
	}

	command, err := jsonObject.Get("cmd").String()
	if err != nil {
		fmt.Println("JSON包未发现COMMAND命令头：", err)
		client.m_conn.Close()
		OnLogout(client, nil)
		return
	}
	switch command {
	case "handSharke1":
		OnHandSharke1(client, jsonObject)
	case "handSharke2":
		OnHandSharke2(client, jsonObject)
	case "ping":
		OnPing(client, jsonObject)
	case "loginWithRoomNo":
		OnLoginWithNo(client, jsonObject)
	case "login":
		OnLogin(client, jsonObject)
	case "enterRoom":
		OnEnterRoom(client, jsonObject)
	case "getRoomInfo":
		OnGetRoomInfo(client, jsonObject)
	case "getRoomList":
		OnGetRoomList(client, jsonObject)
	case "getRoomUserList":
		OnGetRoomUserList(client, jsonObject)
	case "logout":
		OnLogout(client, jsonObject)
	case "leaveRoom":
		OnLeaveRoom(client, jsonObject)
	case "setAdmin":
		OnSetAdmin(client, jsonObject)
	case "setSpeakMode":
		OnSetSpeakMode(client, jsonObject)
	case "setRoomMode":
		OnSetRoomMode(client, jsonObject)
	case "textChat":
		OnTextChat(client, jsonObject)
	case "transparentCmd":
		OnTransparentCmd(client, jsonObject)
	case "openDoc":
		OnOpenDoc(client, jsonObject)
	case "closeDoc":
		OnCloseDoc(client, jsonObject)
	default:
		fmt.Println(command)

	}
}

//从数据库取配置项目

func GetServerConfigFromDB() {

	stmt, err := db.Prepare("Update zl_meeting_room SET onlineUser=0 ")
	checkErr(err)
	_, err = stmt.Exec()
	checkErr(err)
	row := db.QueryRow("SELECT MediaHost,MixAudioPort,RtpMediaPort,BaseUrl,Version FROM zl_config")
	if row == nil {
		fmt.Println("no config in db!")
		MediaHost = ""
		Version = 0
	} else {

		err = row.Scan(&MediaHost, &MixAudioPort, &RtpMediaPort, &BaseUrl, &Version)
		if err != nil {
			fmt.Println("Get Config from db error")
		}

	}
}

func OnGetRoomList(client *Client, jsonObject *JSON.Json) {

	roomInfoCmd := JSON.New()
	roomInfoCmd.Set("cmd", "getRoomList_ret")
	roomInfoCmd.Set("status", 0)
	//查询数据库

	rows, err := db.Query("SELECT roomID,roomName,adminPassword,roomPassword,audioType,isPublic,mixAudio,defaultVideoSize," +
		"  audioSamplerate,UNIX_TIMESTAMP(startTime),UNIX_TIMESTAMP(endTime),roomType,maxUser,bigVideoUser,onlineUser FROM zl_meeting_room")

	checkErr(err)
	//查询
	roomList := make([]*JSON.Json, 0)
	for rows.Next() {
		var (
			roomID, audioType, isPublic, mixAudio, defaultVideoSize int
			audioSamplerate, roomType, maxUser, onlineUser          int
			startTime, endTime                                      uint32
			bigVideoUser                                            int64
			roomName, adminPassword, roomPassword                   string
		)

		err = rows.Scan(&roomID, &roomName, &adminPassword, &roomPassword, &audioType,
			&isPublic, &mixAudio, &defaultVideoSize, &audioSamplerate, &startTime,
			&endTime, &roomType, &maxUser, &bigVideoUser, &onlineUser)
		checkErr(err)

		roomInfo := JSON.New()
		roomInfo.Set("roomID", roomID)
		roomInfo.Set("roomName", roomName)
		roomInfo.Set("adminPassword", adminPassword)
		roomInfo.Set("roomPassword", roomPassword)
		roomInfo.Set("audioType", audioType)
		roomInfo.Set("isPublic", isPublic)
		roomInfo.Set("mixAudio", mixAudio)
		roomInfo.Set("defaultVideoSize", defaultVideoSize)
		roomInfo.Set("audioSamplerate", audioSamplerate)
		roomInfo.Set("startTime", startTime)
		roomInfo.Set("endTime", endTime)
		roomInfo.Set("roomType", roomType)
		roomInfo.Set("maxUser", maxUser)
		roomInfo.Set("bigVideoUser", bigVideoUser)
		roomInfo.Set("onlineUser", onlineUser)
		roomList = append(roomList, roomInfo)
	}

	roomInfoCmd.Set("result", roomList)
	b, _ := roomInfoCmd.MarshalJSON()
	fmt.Println(string(b))
	client.m_conn.Write(append(b, 13, 10))
}
func OnGetRoomInfo(client *Client, jsonObject *JSON.Json) {

	paraRoomID, err := jsonObject.Get("roomID").Int()
	if err != nil {
		return
	}
	roomInfoCmd := JSON.New()
	roomInfoCmd.Set("cmd", "getRoomInfo_ret")
	roomInfoCmd.Set("status", 0)
	row := db.QueryRow("SELECT roomID,roomName,adminPassword,roomPassword,audioType,isPublic,mixAudio,defaultVideoSize,"+
		"  audioSamplerate,UNIX_TIMESTAMP(startTime),UNIX_TIMESTAMP(endTime),roomType,maxUser,bigVideoUser,onlineUser FROM zl_meeting_room where roomID = ?", paraRoomID)

	var (
		roomID, audioType, isPublic, mixAudio, defaultVideoSize int
		audioSamplerate, roomType, maxUser, onlineUser          int
		startTime, endTime                                      uint32
		bigVideoUser                                            int64
		roomName, adminPassword, roomPassword                   string
	)

	err = row.Scan(&roomID, &roomName, &adminPassword, &roomPassword, &audioType,
		&isPublic, &mixAudio, &defaultVideoSize, &audioSamplerate, &startTime,
		&endTime, &roomType, &maxUser, &bigVideoUser, &onlineUser)
	checkErr(err)

	roomInfo := JSON.New()
	roomInfo.Set("roomID", roomID)
	roomInfo.Set("roomName", roomName)
	roomInfo.Set("adminPassword", adminPassword)
	roomInfo.Set("roomPassword", roomPassword)
	roomInfo.Set("audioType", audioType)
	roomInfo.Set("isPublic", isPublic)
	roomInfo.Set("mixAudio", mixAudio)
	roomInfo.Set("defaultVideoSize", defaultVideoSize)
	roomInfo.Set("audioSamplerate", audioSamplerate)
	roomInfo.Set("startTime", startTime)
	roomInfo.Set("endTime", endTime)
	roomInfo.Set("roomType", roomType)
	roomInfo.Set("maxUser", maxUser)
	roomInfo.Set("bigVideoUser", bigVideoUser)
	roomInfo.Set("onlineUser", onlineUser)

	roomInfoCmd.Set("result", roomInfo)
	var (
		shareDoc, docIndex, docTotal int
	)
	shareDoc = 0
	docIndex = 0
	docName := ""
	roomInfo2, ok := roomList[client.m_currentRoomID]
	if !ok {
		fmt.Println("not find room,OnCloseDoc")
	} else {
		shareDoc = roomInfo2.shareDoc
		docIndex = roomInfo2.docIndex
		docTotal = roomInfo2.docTotal
		docName = roomInfo2.docName
	}
	roomInfoCmd.Set("shareDoc", shareDoc)
	if shareDoc == 1 {
		roomInfoCmd.Set("docIndex", docIndex)
		roomInfoCmd.Set("docTotal", docTotal)
		roomInfoCmd.Set("docName", docName)
	}

	b, _ := roomInfoCmd.MarshalJSON()
	fmt.Println(string(b))
	client.m_conn.Write(append(b, 13, 10))
}

func OnEnterRoom(client *Client, jsonObject *JSON.Json) {
	var bParaError bool = false
	paraRoomID, err := jsonObject.Get("roomID").Int()
	if err != nil {

		bParaError = true
	}

	userName, err := jsonObject.Get("userName").String()
	if err != nil {
		bParaError = true
	}

	role, err := jsonObject.Get("userRole").Int()
	if err != nil {
		bParaError = true
	}
	if bParaError {
		strRet := []byte("{\"cmd\":\"enterRoom_ret\",\"status\":1\"msg\":\"failed\",\"userRole\":2}\r\n")
		client.m_conn.Write(strRet)
		return
	}

	bUserIsFull := false
	row := db.QueryRow("SELECT maxUser,onlineUser FROM zl_meeting_room WHERE roomID=?", paraRoomID)
	if row != nil {
		var (
			maxUser, onlineUser int
		)
		err = row.Scan(&maxUser, &onlineUser)
		if err == nil {
			if onlineUser > maxUser-1 {
				bUserIsFull = true
				fmt.Println("room user full")
			}
		} else {
			bUserIsFull = true
			fmt.Println("db error:", paraRoomID)
		}
	} else {
		bUserIsFull = true
		fmt.Println("select error")
	}

	if bUserIsFull {
		strRet := []byte("{\"cmd\":\"enterRoom_ret\",\"status\":2\"msg\":\"user full\"}\r\n")

		client.m_conn.Write(strRet)
		return
	}

	strRet := []byte("{\"cmd\":\"enterRoom_ret\",\"status\":0,\"msg\":\"success\",\"userRole\":2}\r\n")
	fmt.Println("OnEnterRoom", string(strRet))
	client.m_conn.Write(strRet)
	client.m_userName = userName
	client.m_userRole = role
	client.m_currentRoomID = paraRoomID

	//通知房间内其它成员，我上线了

	cmdObj := JSON.New()
	cmdObj.Set("cmd", "transparentCmd")

	cmdObj.Set("fromSessionID", client.m_sessionID)
	cmdObj.Set("toSessionID", 0)
	cmdObj.Set("fromUserName", client.m_userName)
	cmdObj.Set("roomID", client.m_currentRoomID)
	subObj := JSON.New()
	subObj.Set("cmd", "userOnline")
	subObj.Set("sessionID", client.m_sessionID)
	subObj.Set("userName", client.m_userName)
	subObj.Set("userRole", client.m_userRole)
	subObj.Set("clientType", client.m_clientType)

	cmdObj.Set("subCmd", subObj)
	OnTransparentCmd(client, cmdObj)
	//更新房间在线用户
	stmt, err := db.Prepare("Update zl_meeting_room SET onlineUser=onlineUser+1 where roomID = ?")
	checkErr(err)
	_, err = stmt.Exec(client.m_currentRoomID)
	checkErr(err)

}

func OnLeaveRoom(client *Client, jsonObject *JSON.Json) {
	//通知房间内其它成员，我下线了
	roomInfo, ok := roomList[client.m_currentRoomID]
	if !ok {
		fmt.Println("not find room,OnCloseDoc")
	} else {
		if roomInfo.shareDoc == 1 && roomInfo.shareDocUser == client.m_sessionID {
			//通知其它用户关闭文档
			roomInfo.shareDoc = 0
			cmdObj := JSON.New()
			cmdObj.Set("cmd", "transparentCmd")
			cmdObj.Set("fromSessionID", client.m_sessionID)
			cmdObj.Set("toSessionID", 0)
			cmdObj.Set("fromUserName", client.m_userName)
			cmdObj.Set("roomID", client.m_currentRoomID)
			subCmd := JSON.New()
			subCmd.Set("cmd", "closeDoc")
			cmdObj.Set("subCmd", subCmd)
			OnTransparentCmd(client, cmdObj)
		}
	}

	cmdObj := JSON.New()
	cmdObj.Set("cmd", "transparentCmd")

	cmdObj.Set("fromSessionID", client.m_sessionID)
	cmdObj.Set("toSessionID", 0)
	cmdObj.Set("fromUserName", client.m_userName)
	cmdObj.Set("roomID", client.m_currentRoomID)
	subObj := JSON.New()
	subObj.Set("cmd", "userOffline")
	subObj.Set("sessionID", client.m_sessionID)
	cmdObj.Set("subCmd", subObj)
	fmt.Println("On leave room")
	OnTransparentCmd(client, cmdObj)
	subtractOnlineUser(client.m_currentRoomID)
	client.m_currentRoomID = 0
}

//用户退出房间，更新在线用户
func subtractOnlineUser(roomID int) {
	row := db.QueryRow("SELECT onlineUser FROM zl_meeting_room where roomID=?", roomID)
	if row != nil {
		var (
			onlineUser int
		)
		err = row.Scan(&onlineUser)
		if err == nil {
			if onlineUser > 0 {
				stmt, err := db.Prepare("Update zl_meeting_room SET onlineUser=onlineUser-1 where roomID = ?")
				checkErr(err)
				_, err = stmt.Exec(roomID)
				checkErr(err)
			} else {
				stmt, err := db.Prepare("Update zl_meeting_room SET onlineUser=0 where roomID = ?")
				checkErr(err)
				_, err = stmt.Exec(roomID)
				checkErr(err)
			}
		}
	}
}

//检查房间存不存在，检查房间密码是否存在
func OnLoginWithNo(client *Client, jsonObject *JSON.Json) {

	cmdObj := JSON.New()
	cmdObj.Set("cmd", "loginWithRoomNo_ret")

	cmdObj.Set("status", 0)
	cmdObj.Set("msg", "enter room success")

	paraRoomID, err := jsonObject.Get("roomID").Int()
	if err != nil {
		cmdObj.Set("status", 1)
		cmdObj.Set("msg", "roomID parameter is null")
	}
	roomPassword2, err := jsonObject.Get("password").String()
	if err != nil {
		cmdObj.Set("status", 1)
		cmdObj.Set("msg", "password parameter is null")
	}

	row := db.QueryRow("SELECT roomPassword FROM zl_meeting_room where roomID = ?", paraRoomID)
	if row == nil {
		cmdObj.Set("status", 2)
		cmdObj.Set("msg", "not find this room")
	} else {
		var roomPassword string
		err = row.Scan(&roomPassword)
		if err != nil {
			cmdObj.Set("status", 3)
			cmdObj.Set("msg", "db error")
		} else {
			if roomPassword != roomPassword2 {
				cmdObj.Set("status", 4)
				cmdObj.Set("msg", "password is not correct")
			}
		}
	}
	if Version > 0 {
		config := JSON.New()
		config.Set("MediaHost", MediaHost)
		config.Set("MixAudioPort", MixAudioPort)
		config.Set("RtpMediaPort", RtpMediaPort)
		config.Set("BaseUrl", BaseUrl)
		cmdObj.Set("config", config)
	}

	b, _ := cmdObj.MarshalJSON()
	fmt.Println(string(b))
	client.m_conn.Write(append(b, 13, 10))
}

//检查用户名，密码是否正确
func OnLogin(client *Client, jsonObject *JSON.Json) {
	cmdObj := JSON.New()
	cmdObj.Set("cmd", "login_ret")

	cmdObj.Set("status", 1)
	cmdObj.Set("msg", "login failed")

	paraAccount, err := jsonObject.Get("account").String()
	if err != nil {
		cmdObj.Set("status", 1)
		cmdObj.Set("msg", "account parameter is null")
	}
	paraPassword, err := jsonObject.Get("password").String()
	if err != nil {
		cmdObj.Set("status", 1)
		cmdObj.Set("msg", "password parameter is null")
	}

	row := db.QueryRow("SELECT userID,userName,password FROM zl_user where account = ?", paraAccount)
	if row == nil {
		cmdObj.Set("status", 2)
		cmdObj.Set("msg", "not find this user")
	} else {
		var (
			userID   int
			userName string
			password string
		)
		err = row.Scan(&userID, &userName, &password)
		if err != nil {
			cmdObj.Set("status", 3)
			cmdObj.Set("msg", "db error")
		} else {
			if password == paraPassword {
				cmdObj.Set("status", 0)
				cmdObj.Set("userID", userID)
				cmdObj.Set("userName", userName)
				cmdObj.Set("msg", "login success")
				if Version > 0 {
					config := JSON.New()
					config.Set("MediaHost", MediaHost)
					config.Set("MixAudioPort", MixAudioPort)
					config.Set("RtpMediaPort", RtpMediaPort)
					config.Set("BaseUrl", BaseUrl)
					cmdObj.Set("config", config)
				}
			}
		}
	}
	b, _ := cmdObj.MarshalJSON()
	fmt.Println(string(b))
	client.m_conn.Write(append(b, 13, 10))
}

//登出
func OnLogout(client *Client, jsonObject *JSON.Json) {
	//fmt.Println("logout from server", client.m_sessionID, client.m_currentRoomID)
	mutex.Lock()
	defer mutex.Unlock()
	if client.m_sessionID > 0 {

		//从用户列表中删除
		if client.m_currentRoomID > 0 {
			OnLeaveRoom(client, jsonObject)
		}
		delete(clientList, client.m_sessionID)
	}
	client.m_currentRoomID = 0
}

//心跳包
func OnPing(client *Client, jsonObject *JSON.Json) {
	fmt.Println("OnPing", "{\"cmd\":\"pong\"}\r\n")
	client.m_conn.Write([]byte("{\"cmd\":\"pong\"}\r\n"))
}

//握手一
func OnHandSharke1(client *Client, jsonObject *JSON.Json) {
	cmdVar, err := jsonObject.Get("cmdVar").Int()
	if err != nil {
		return
	}
	if cmdVar != Version {
		cmdObj := JSON.New()
		cmdObj.Set("cmd", "handSharke1_ret")
		cmdObj.Set("status", 1)
		cmdObj.Set("msg", "command version not correct")
		cmdObj.Set("updateUrl", BaseUrl+"/setup.exe")
		b, _ := cmdObj.MarshalJSON()
		fmt.Println(string(b))
		client.m_conn.Write(append(b, 13, 10))
	} else {
		token := strconv.FormatInt(time.Now().UnixNano(), 10)
		strRet := []byte("{\"cmd\":\"handSharke1_ret\",\"status\":0,\"token\":\"" + token + "\"}\r\n")
		client.m_conn.Write(strRet)
	}
}

//握手二
func OnHandSharke2(client *Client, jsonObject *JSON.Json) {
	var sessionID int64 = time.Now().UnixNano()
	client.m_sessionID = sessionID
	cmdObj := JSON.New()
	cmdObj.Set("cmd", "handSharke2_ret")
	cmdObj.Set("status", 0)
	cmdObj.Set("msg", "login success")
	cmdObj.Set("sessionID", sessionID)
	mutex.Lock()
	defer mutex.Unlock()
	clientList[sessionID] = client
	b, _ := cmdObj.MarshalJSON()
	fmt.Println(string(b))
	client.m_conn.Write(append(b, 13, 10))
}

func OnGetRoomUserList(client *Client, jsonObject *JSON.Json) {
	cmdObj := JSON.New()
	cmdObj.Set("cmd", "getRoomUserList_ret")
	cmdObj.Set("status", 0)
	userList := make([]*JSON.Json, 0)
	roomID := client.m_currentRoomID
	mutex.Lock()
	defer mutex.Unlock()
	for _, v := range clientList {
		if v.m_currentRoomID == roomID {
			userInfo := JSON.New()
			userInfo.Set("sessionID", v.m_sessionID)
			userInfo.Set("userName", v.m_userName)
			userInfo.Set("userRole", v.m_userRole)
			userInfo.Set("clientType", 1)
			userList = append(userList, userInfo)
		}
	}
	cmdObj.Set("result", userList)
	b, _ := cmdObj.MarshalJSON()
	fmt.Println(string(b))
	client.m_conn.Write(append(b, 13, 10))
}

func OnSetAdmin(client *Client, jsonObject *JSON.Json) {
	paraSessionID, err := jsonObject.Get("sessionID").Int64()
	if err != nil {
		return
	}
	bSet, err := jsonObject.Get("bSet").Int()
	if err != nil {
		return
	}
	mutex.Lock()
	defer mutex.Unlock()
	client, ok := clientList[paraSessionID]
	if !ok {
		fmt.Println("not find user,OnSetAdmin")
		return
	}
	if bSet == 0 {
		client.m_userRole = 2
	} else {
		client.m_userRole = 1
	}
}

func OnSetSpeakMode(client *Client, jsonObject *JSON.Json) {
	mode, err := jsonObject.Get("mode").Int()
	if err != nil {
		return
	}
	if client.m_currentRoomID == 0 {
		return
	}
	//更新数据库
	stmt, err := db.Prepare("UPDATE zl_meeting_room set audioType = ? where roomID = ?")
	checkErr(err)
	_, err = stmt.Exec(mode, client.m_currentRoomID)
	checkErr(err)
}

func OnSetRoomMode(client *Client, jsonObject *JSON.Json) {
	mode, err := jsonObject.Get("mode").Int()
	if err != nil {
		return
	}
	if client.m_currentRoomID == 0 {
		return
	}
	//更新数据库
	stmt, err := db.Prepare("UPDATE zl_meeting_room set roomType = ? where roomID = ?")
	checkErr(err)
	_, err = stmt.Exec(mode, client.m_currentRoomID)
	checkErr(err)
}
func OnTextChat(client *Client, jsonObject *JSON.Json) {
	toSessionID, err := jsonObject.Get("toSessionID").Int64()
	if err != nil {
		return
	}

	fromSessionID, err := jsonObject.Get("fromSessionID").Int64()
	if err != nil {
		return
	}
	roomID := client.m_currentRoomID
	jsonObject.Set("date", time.Now().Unix())
	//指定用户发送
	mutex.Lock()
	defer mutex.Unlock()
	if toSessionID != 0 {
		u, ok := clientList[toSessionID]
		if ok {
			//找到这个用户
			b, _ := jsonObject.MarshalJSON()
			fmt.Println(string(b))
			u.m_conn.Write(append(b, 13, 10))
		}
	} else {
		//房间内成员全部发送
		for _, u := range clientList {
			if u.m_currentRoomID == roomID && u.m_sessionID != fromSessionID {
				b, _ := jsonObject.MarshalJSON()
				fmt.Println(string(b))
				u.m_conn.Write(append(b, 13, 10))
			}
		}
	}
}

func OnOpenDoc(client *Client, jsonObject *JSON.Json) {
	//保存房间属性
	roomInfo, ok := roomList[client.m_currentRoomID]
	if !ok {
		//fmt.Println("not find user,OnSetAdmin")
		roomInfo = new(RoomInfo)
		roomList[client.m_currentRoomID] = roomInfo
		//return
	}
	docIndex, err := jsonObject.Get("index").Int()
	if err != nil {
		fmt.Println("OnOpenDoc index is null")
		return
	}

	docTotal, err := jsonObject.Get("total").Int()
	if err != nil {
		fmt.Println("OnOpenDoc total is null")
		return
	}

	docName, err := jsonObject.Get("url").String()
	if err != nil {
		fmt.Println("OnOpenDoc url is null")
		return
	}

	roomInfo.shareDoc = 1
	roomInfo.shareDocUser = client.m_sessionID
	roomInfo.docIndex = docIndex
	roomInfo.docTotal = docTotal
	roomInfo.docName = docName
	//广播通知
	cmdObj := JSON.New()
	cmdObj.Set("cmd", "transparentCmd")
	cmdObj.Set("fromSessionID", client.m_sessionID)
	cmdObj.Set("toSessionID", 0)
	cmdObj.Set("fromUserName", client.m_userName)
	cmdObj.Set("roomID", client.m_currentRoomID)
	cmdObj.Set("subCmd", jsonObject)
	fmt.Println("OnOpenDoc")
	OnTransparentCmd(client, cmdObj)
}

func OnCloseDoc(client *Client, jsonObject *JSON.Json) {

	roomInfo, ok := roomList[client.m_currentRoomID]
	if !ok {
		fmt.Println("not find room,OnCloseDoc")
	} else {
		roomInfo.shareDoc = 0
	}

	//广播通知
	cmdObj := JSON.New()
	cmdObj.Set("cmd", "transparentCmd")
	cmdObj.Set("fromSessionID", client.m_sessionID)
	cmdObj.Set("toSessionID", 0)
	cmdObj.Set("fromUserName", client.m_userName)
	cmdObj.Set("roomID", client.m_currentRoomID)
	cmdObj.Set("subCmd", jsonObject)
	fmt.Println("OnCloseDoc")
	OnTransparentCmd(client, cmdObj)
}

//转发透明命令
func OnTransparentCmd(client *Client, jsonObject *JSON.Json) {

	toSessionID, err := jsonObject.Get("toSessionID").Int64()
	if err != nil {
		//fmt.Println("OnTransparentCmd: toSessionID is null")
		return
	}

	fromSessionID, err := jsonObject.Get("fromSessionID").Int64()
	if err != nil {
		fmt.Println("OnTransparentCmd: fromSessionID is null")
		return
	}

	roomID := client.m_currentRoomID

	//指定用户发送
	//mutex.Lock()
	//fmt.Println("OnTransparentcmd:", fromSessionID, toSessionID, roomID)
	if toSessionID != 0 {
		u, ok := clientList[toSessionID]
		if ok {
			//找到这个用户
			b, _ := jsonObject.MarshalJSON()

			u.m_conn.Write(append(b, 13, 10))
		}
	} else {
		//房间内成员全部发送
		for _, u := range clientList {
			if u.m_currentRoomID == roomID && u.m_sessionID != fromSessionID {
				b, _ := jsonObject.MarshalJSON()
				//fmt.Println(string(b))
				u.m_conn.Write(append(b, 13, 10))
			}
		}
	}
	//mutex.Unlock()
}

func checkErr(err error) {
	if err != nil {
		panic(err)
	}
}

func main() {
	fmt.Println("Start Meeting Server!")
	runtime.GOMAXPROCS(runtime.NumCPU())
	//读取配置文件
	configFile := "config.json"
	fin, err := os.Open(configFile)
	defer fin.Close()
	if err != nil {
		fmt.Println(configFile, err)
		return
	}
	buf := make([]byte, 1024)
	n, _ := fin.Read(buf)
	if 0 == n {
		fmt.Println("config file format error")
		return
	}

	jsonObject, err := JSON.NewJson(buf[:n])
	if err != nil {
		fmt.Println("解析JSON异常：", err)
		return
	}
	port, err := jsonObject.Get("PORT").Int()
	checkErr(err)
	dbHost, err := jsonObject.Get("DBHOST").String()
	checkErr(err)
	dbName, err := jsonObject.Get("DBNAME").String()
	checkErr(err)
	dbPassword, err := jsonObject.Get("DBPASSWORD").String()
	checkErr(err)
	dbUser, err := jsonObject.Get("DBUSER").String()
	checkErr(err)
	strDB := fmt.Sprintf("%s:%s@tcp(%s)/%s?charset=utf8", dbUser, dbPassword, dbHost, dbName)
	fmt.Println(strDB)
	db, err = sql.Open("mysql", strDB)
	checkErr(err)
	db.SetMaxOpenConns(500)
	db.SetMaxIdleConns(50)
	db.Ping()
	fmt.Println("connect to db sucess")
	ip := net.ParseIP("*")
	addr := net.TCPAddr{IP: ip, Port: port}
	listener, err := net.ListenTCP("tcp", &addr)
	if err != nil {
		fmt.Println("server start failure", err)
		return
	}
	fmt.Println("Meeting server start ok!")
	GetServerConfigFromDB()
	//日志
	//监听
	for {
		conn, err := listener.AcceptTCP()
		if err != nil {
			if ne, ok := err.(net.Error); ok && ne.Temporary() {
				continue
			}
			return
		}
		go ServiceClient(conn)
	}
	fmt.Println("server has quit,press any key to quit!")
	input := make([]byte, 1024)
	os.Stdin.Read(input)
	db.Close()
}
