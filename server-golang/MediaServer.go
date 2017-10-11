package main

import (
	"encoding/binary"
	"fmt"
	"net"
	"os"
	"runtime"
	"strconv"
	"time"
)

var (
	PubStreamList = make(map[uint32]*PubStream, 100) //房间端集合
	udpConn       *net.UDPConn                       //全局UDP连接
	packageChan   chan *PackageChandata              //处理消息包的协程通道
	Ping          = "ping"                           //心跳包命令头
	Pong          = []byte("pong")                   //心跳包回复
)

//抽象一个传输房间
type PubStream struct {
	Chan           chan []byte    //房间处理协程通道
	Ssrc           uint32         //房间唯一标示ID
	LastSeq        uint32         //包系列号
	BufList        [][]byte       //缓存最新的200个包
	RecvList       []*net.UDPAddr //接收方地址集合
	LastActiveTime int64          //最后活动时间
	IsStart        bool           //是否开始
	addr           *net.UDPAddr   //发布流的地址
}

//服务器抽象
type Server struct {
	Port int
}

//处理消息包的协程通道传输的数据类型
type PackageChandata struct {
	Packeage   []byte
	N          int
	ClientAddr *net.UDPAddr
}

//开启服务器监听
func (ser *Server) Start() {
	pUDPAddr, err := net.ResolveUDPAddr("udp", ":"+strconv.Itoa(ser.Port))
	if err != nil {
		fmt.Println(err)
	}
	packageChan = make(chan *PackageChandata, 2000)
	go PackageHandle(packageChan)
	ser.Loop(pUDPAddr)
}

//循环接收
func (ser *Server) Loop(pUDPAddr *net.UDPAddr) {
	conn, err := net.ListenUDP("udp", pUDPAddr)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %s", err.Error())
		os.Exit(1)
	}
	udpConn = conn
	defer udpConn.Close()
	fmt.Println("media server started ok")
	buf := make([]byte, 2500)
	for {
		n, clientAddr, err := udpConn.ReadFromUDP(buf)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error: %s", err.Error())
			continue
		}
		p := make([]byte, n)
		copy(p, buf[:n])
		packageChan <- &PackageChandata{Packeage: p, N: n, ClientAddr: clientAddr}
	}
}

//严重错误处理恢复
func RecoverError() {
	if err := recover(); err != nil {
		fmt.Println(err)
		go PackageHandle(packageChan)
	}
}

//处理包
func PackageHandle(c <-chan *PackageChandata) {
	defer RecoverError()
	for {
		packageChandata := <-c
		packages, clientAddr, n := packageChandata.Packeage, packageChandata.ClientAddr, packageChandata.N
		if n > 12 {
			//fmt.Fprintf(os.Stdout, "readed: %d", len(packages))
			rtpPayloadType := packages[1] & 0x7F //第二个字节
			seq := uint32(binary.BigEndian.Uint16(packages[2:]))
			//timestamp := binary.BigEndian.Uint32(packages[4:])
			ssrc := binary.BigEndian.Uint32(packages[8:])
			//fmt.Println("消息 ", "type:", rtpPayloadType, "seq:", seq, "time:", timestamp, "sid:", ssrc)
			if rtpPayloadType == 127 && n >= 24 { //操作命令
				rtpCommand := binary.LittleEndian.Uint32(packages[12:])
				//recvSSRC := binary.LittleEndian.Uint32(packages[16:])
				//roomID := binary.LittleEndian.Uint32(packages[20:])
				//fmt.Println("消息 ", "type:", rtpPayloadType, "rtpCommand:", rtpCommand, "seq:", seq, "time:", timestamp, "sid:", ssrc, "recvSSRC:", recvSSRC, " roomID:", roomID)
				switch rtpCommand {
				case 0:
					OnPublishStart(ssrc, clientAddr, 0)
				case 1:
					OnPublishStop(ssrc)
				case 2:
					OnRecvStart(ssrc, clientAddr)
				case 3:
					OnRecvStop(ssrc, clientAddr)
				case 4:
					//暂时屏蔽重传
					OnReSend(ssrc, packages, clientAddr)
				default:
					fmt.Printf("invalide command:%d \r\n", rtpCommand)
				}
				OnCommandAck(ssrc, clientAddr)
			} else if rtpPayloadType == 0 || rtpPayloadType == 19 { //传输命令
				pubStream := PubStreamList[ssrc]
				if pubStream != nil {
					pubStream.Chan <- packages
				} else { //恢复
					OnPublishStart(ssrc, clientAddr, seq)
				}
			} else {
				fmt.Println("not find rtptype, rtpPayloadType:", rtpPayloadType, " len:", n)
			}
		} else {
			msg := string(packages)
			if msg == Ping {
				udpConn.WriteToUDP(Pong, clientAddr)
			} else {
				fmt.Println("invalide packet!!! from ")
			}
		}
	}
}

//处理数据包传输
func PackageTranslate(clientAddr *net.UDPAddr, c chan []byte) {
	for {
		packages := <-c
		if packages != nil {
			rtpPayloadType := packages[1] & 0x7F //第二个字节
			seq := uint32(binary.BigEndian.Uint16(packages[2:]))
			timestamp := binary.BigEndian.Uint32(packages[4:])
			ssrc := binary.BigEndian.Uint32(packages[8:])
			//fmt.Println("处理消息 ", "seq:", seqTemp,"sid:", ssrc)
			//silk 8khz
			if rtpPayloadType != 19 {
				seq = timestamp / 20
			}
			//转发
			pubStream := PubStreamList[ssrc]
			if pubStream != nil {
				pubStream.LastActiveTime = time.Now().Unix()
				//缓存到音频缓存
				if seq == pubStream.LastSeq {
					//正常
					pubStream.LastSeq++
				} else if seq > pubStream.LastSeq {
					//检测到丢包
					count := seq - pubStream.LastSeq
					//console.log('lost packet start: '+ seq +' count:' + count+'lastSeq:'+pubStream.lastSeq);
					//通知客户端重传
					cmdBuf := make([]byte, 32)
					cmdBuf[1] = 127
					binary.BigEndian.PutUint32(cmdBuf[8:], ssrc)
					binary.LittleEndian.PutUint32(cmdBuf[12:], 4)
					binary.LittleEndian.PutUint32(cmdBuf[24:], pubStream.LastSeq)
					binary.LittleEndian.PutUint32(cmdBuf[28:], uint32(count))
					fmt.Println("...丢包", clientAddr.IP.String(), "count:", count, "lastseq:", pubStream.LastSeq, "curseq:", seq)
					udpConn.WriteToUDP(cmdBuf, clientAddr)
					pubStream.LastSeq = seq + 1
				} else {
					//过期的包，可能是重传过来的
				}

				/****/
				pubStream.BufList = append(pubStream.BufList, packages)
				if len(pubStream.BufList) > 200 {
					pubStream.BufList = pubStream.BufList[1:]
				}

				//故意丢包
				//if(seq%10 == 0)
				//return;
				for _, addr := range pubStream.RecvList {
					if addr != nil {
						_, err := udpConn.WriteToUDP(packages, addr)
						if err != nil {
							OnRecvStop(ssrc, addr)
						}
					}
				}
			}
		} else {
			break
		}
	}
}

//发送端接入
func OnPublishStart(ssrc uint32, clientAddr *net.UDPAddr, seq uint32) {
	fmt.Println("start publish ssrc:", ssrc)
	pubStream := PubStreamList[ssrc]
	if pubStream == nil {
		c := make(chan []byte, 200)
		if seq != 0 {
			seq++
		}
		pubStream = &PubStream{Chan: c, Ssrc: ssrc, LastSeq: seq, BufList: make([][]byte, 0, 400), RecvList: make([]*net.UDPAddr, 0, 10), LastActiveTime: time.Now().Unix(), addr: clientAddr}
		PubStreamList[ssrc] = pubStream
	}
	pubStream.addr = clientAddr
	go PackageTranslate(clientAddr, pubStream.Chan)
	pubStream.IsStart = true
}

//发送端退出
func OnPublishStop(ssrc uint32) {
	pubStream := PubStreamList[ssrc]
	if pubStream != nil {
		pubStream.Chan <- nil
		if len(pubStream.RecvList) == 0 {
			DelPubstream(ssrc, pubStream)
		} else {
			fmt.Println("发送端退出，还有接收端，不清理房间")
			pubStream.IsStart = false
		}
		fmt.Println("stop publish ssrc:", ssrc, "len:", len(PubStreamList))
	}
}

//接收端接入
func OnRecvStart(ssrc uint32, clientAddr *net.UDPAddr) {
	pubStream := PubStreamList[ssrc]
	if pubStream != nil {
		//fmt.Println("start resv ssrc:", ssrc)
		hasFind := false
		for _, addr := range pubStream.RecvList {
			if addr != nil {
				if clientAddr.String() == addr.String() {
					hasFind = true
					break
				}
			}
		}

		if !hasFind {
			pubStream.RecvList = append(pubStream.RecvList, clientAddr)
			fmt.Println("add recv stream to :", ssrc)
			//给这个流发送开始发数据指令
			cmdBuf := make([]byte, 24)
			cmdBuf[1] = 127
			binary.BigEndian.PutUint32(cmdBuf[8:], ssrc)
			binary.LittleEndian.PutUint32(cmdBuf[12:], 7)
			udpConn.WriteToUDP(cmdBuf, pubStream.addr)
		}
	} else { //发送方还未开始，接收方先进来等待
		//fmt.Println("wait publish ssrc:", ssrc)
		c := make(chan []byte, 200)
		recvlist := make([]*net.UDPAddr, 1, 10)
		recvlist[0] = clientAddr
		pubStream := PubStream{Chan: c, Ssrc: ssrc, LastSeq: 0, BufList: make([][]byte, 0), RecvList: recvlist, LastActiveTime: time.Now().Unix()}
		PubStreamList[ssrc] = &pubStream
		fmt.Println("add recv stream to :", ssrc)
	}
}

//接收端退出
func OnRecvStop(ssrc uint32, clientAddr *net.UDPAddr) {
	pubStream := PubStreamList[ssrc]
	if pubStream != nil {
		hasDelPub := false
		if !pubStream.IsStart {
			if len(pubStream.RecvList) <= 0 {
				DelPubstream(ssrc, pubStream)
				hasDelPub = true
			}
		}
		if !hasDelPub {

			for i, addr := range pubStream.RecvList {
				if addr.String() == clientAddr.String() {
					fmt.Println("stop resv ssrc:", ssrc)
					pubStream.RecvList = append(pubStream.RecvList[:i], pubStream.RecvList[(i+1):]...)
					break
				}
			}

			if len(pubStream.RecvList) == 0 {
				//没有人接收这个流，通知它可以暂停发送数据了
				cmdBuf := make([]byte, 24)
				cmdBuf[1] = 127
				binary.BigEndian.PutUint32(cmdBuf[8:], ssrc)
				binary.LittleEndian.PutUint32(cmdBuf[12:], 6)
				udpConn.WriteToUDP(cmdBuf, pubStream.addr)
			}
		}
	}
}

//删除房间
func DelPubstream(ssrc uint32, pubStream *PubStream) {
	pubStream.RecvList = nil
	pubStream.BufList = nil
	pubStream.Ssrc = 0
	pubStream = nil
	delete(PubStreamList, ssrc)
}

//处理客户端发过来的重传数据包请求
func OnReSend(ssrc uint32, packages []byte, clientAddr *net.UDPAddr) {
	pubStream := PubStreamList[ssrc]
	if pubStream != nil {
		firstSeq := binary.LittleEndian.Uint32(packages[24:])
		fmt.Println("重传：", firstSeq)
		//count := binary.LittleEndian.Uint32(packages[28:])
		for _, buf := range pubStream.BufList {
			if buf != nil {
				//rtpPayloadType2 := buf[1] & 0x7F //第二个字节
				seq2 := binary.BigEndian.Uint16(buf[2:])
				if uint32(seq2) == firstSeq {
					//fmt.Printf("len: %d type,%d, seq:%d", len(buf), rtpPayloadType2, seq2)
					udpConn.WriteToUDP(buf, clientAddr)

					fmt.Println("重传：", firstSeq)
					break
				}
			}
		}
	}
}

//发送消息确认包
func OnCommandAck(ssrc uint32, clientAddr *net.UDPAddr) {
	cmdBuf := make([]byte, 24)
	cmdBuf[1] = 127
	binary.BigEndian.PutUint32(cmdBuf[8:], ssrc)
	binary.LittleEndian.PutUint32(cmdBuf[12:], 5)
	udpConn.WriteToUDP(cmdBuf, clientAddr)
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	fmt.Println("media server start...")
	fmt.Println("pid: ", os.Getpid())
	ser := Server{Port: 7084}
	ser.Start()
}
