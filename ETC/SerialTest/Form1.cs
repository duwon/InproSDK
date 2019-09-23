using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;

namespace SerialTest
{
    public partial class SerialTest : Form
    {
        enum Stage : byte { START = 0, MESSAGETYPE = 1, DATA = 2, PARSING = 4, CHECKSUM = 3, SEND = 5, RESEND = 6, STOP = 7, WRITE = 8, END = 9, LENGTH = 10 };

        public struct Message
        {
            public byte nextStage;
            public byte type;
            public byte destID;
            public byte srcID;
            public byte payloadSize;
            public byte packetSize;
            public byte dataCount;
            public byte checksum;
            public byte startFlag;
            public byte[] data;

            public Message(byte[] data)
            {
                this.nextStage = 0;
                this.type = 0;
                this.payloadSize = 0;
                this.packetSize = 0;
                this.checksum = 0;
                this.data = data;
                destID = 0;
                srcID = 0;
                dataCount = 5;
                startFlag = 0;
            }
        }

        enum Packet : byte { STX = 0xA5, ETX = 0x04, CHECKSUM = 0x00, DATA = 0x00, MAX_LENGTH = 0xf0, ERROR = 0x00, SUCCESS = 0x01 };
        System.Collections.Concurrent.ConcurrentQueue<byte> rxBuffer = new System.Collections.Concurrent.ConcurrentQueue<byte>();
        Message RxMessage = new Message();
        DataPacket packet = new DataPacket();

        public SerialTest()
        {
            InitializeComponent();

            CheckForIllegalCrossThreadCalls = false;

            //패킷 데이터 저장 갯수 생성
            RxMessage.data = new byte[(byte)Packet.MAX_LENGTH];

            //컴포트 버튼 초기화
            spButtonControl(false);

            //컴포트  읽어오기
            try
            {
                comboBox_PortName.Items.AddRange(SerialPort.GetPortNames());
            }
            catch { }

            if (comboBox_PortName.Items.Count > 0)
            {
                comboBox_PortName.SelectedIndex = 0;
            }

            //GUI 버전정보 업데이트
            label_ProjectVersion.Text = "GUI Version : " + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.Major.ToString() + "." + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.Minor.ToString();

        }

        private void button_COMOpen_Click(object sender, EventArgs e)
        {
            try
            {
                if (!sp1.IsOpen)
                {
                    sp1.PortName = this.comboBox_PortName.Text;
                    sp1.BaudRate = 115200;

                    sp1.RtsEnable = true;
                    sp1.Open();

                    this.spButtonControl(true);
                }
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void button_COMClose_Click(object sender, EventArgs e)
        {
            if (null != sp1)
            {
                if (sp1.IsOpen)
                {
                    sp1.Close();
                }
            }
            this.spButtonControl(false);
        }

        private void comboBox_PortName_MouseDown(object sender, MouseEventArgs e)
        {
            comboBox_PortName.Items.Clear();
            comboBox_PortName.Items.AddRange(SerialPort.GetPortNames());
        }

        private void spButtonControl(bool status)
        {
            button_COMOpen.Enabled = !status;
            button_COMClose.Enabled = status;
        }

        private void sendFrame(byte[] frameData)
        {
            int frameLength = frameData[5] + 8;
            frameData[frameLength - 2] = calCheckSum(frameData, frameLength);

            try
            {
                textBox_RxHex.AppendText("\r\n");
                sp1.Write(frameData, 0, frameLength);
                textBox_RxHex.AppendText(BitConverter.ToString(frameData).Replace("-", " ") + " ");
            }
            catch
            {
            }
        }
        private byte calCheckSum(byte[] frameData, int frameLength)
        {
            byte checksum = 0;
            for (int i = 0; i < (frameLength - 2); i++)
                checksum ^= frameData[i];
            //checksum = (byte)((checksum ^ 0xFF) );
            return checksum;
        }

        private void sp1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                int spRxSize = sp1.BytesToRead; // 수신된 바이트 갯수

                if (spRxSize != 0)              
                {
                    byte[] rxBuff = new byte[spRxSize];

                    try
                    {
                        sp1.Read(rxBuff, 0, spRxSize);

                        //수신 데이터 디버깅 창에 출력
                        textBox_RxHex.AppendText(DateTime.Now.ToString("\r\n[HH:mm:ss] [RX] ") + BitConverter.ToString(rxBuff).Replace("-", " "));

                        string strTemp = this.AddBytes(rxBuff.ToList()).Replace("\r", "").Replace("\n", Environment.NewLine);
                        textBox_Debug.AppendText(strTemp);

                        for (int i = 0; i < spRxSize; i++)
                        {
                            rxBuffer.Enqueue(rxBuff[i]);
                        }
                        ProcessMessage();
                    }
                    catch { }
                }
            }
            catch (System.Exception)
            {
            }
        }

        private void ProcessMessage()
        {
            while ((rxBuffer.Count != 0) || (RxMessage.nextStage == (byte)Stage.PARSING) || (RxMessage.nextStage == (byte)Stage.CHECKSUM))
            {
                switch (RxMessage.nextStage)
                {
                    case (byte)Stage.START:
                        {

                            byte _tch;
                            for (int i = 0; i < rxBuffer.Count; i++)
                            {
                                rxBuffer.TryDequeue(out _tch);
                                if (_tch == (byte)Packet.STX)
                                {
                                    RxMessage.startFlag++;
                                }
                                if (RxMessage.startFlag == 2)
                                {
                                    RxMessage.startFlag = 0;
                                    RxMessage.nextStage = (byte)Stage.MESSAGETYPE;
                                    RxMessage.data[0] = (byte)Packet.STX;
                                    RxMessage.data[1] = (byte)Packet.STX;
                                    break;
                                }
                            }
                            break;
                        }
                    case (byte)Stage.MESSAGETYPE:
                        {
                            rxBuffer.TryDequeue(out RxMessage.data[2]);
                            RxMessage.destID = RxMessage.data[2];

                            rxBuffer.TryDequeue(out RxMessage.data[3]);
                            RxMessage.srcID = RxMessage.data[3];

                            rxBuffer.TryDequeue(out RxMessage.data[4]);
                            RxMessage.payloadSize = RxMessage.data[4];
                            RxMessage.packetSize = (byte)(RxMessage.payloadSize + 7);//15);// 7);
   
                            if (RxMessage.payloadSize != 0)                              
                            {
                                RxMessage.nextStage = (byte)Stage.DATA;             
                            }
                            else                                                    
                            {
                                RxMessage.nextStage = (byte)Stage.START;            /* START 단계로 돌아가기 */
                            }

                            RxMessage.nextStage = (byte)Stage.DATA;
                            break;
                        }
                    case (byte)Stage.DATA:
                        {
                            rxBuffer.TryDequeue(out RxMessage.data[RxMessage.dataCount]);
                            RxMessage.dataCount++;

                            if (RxMessage.dataCount == RxMessage.packetSize)
                            {
                                RxMessage.dataCount = 5;
                                if (RxMessage.data[RxMessage.packetSize - 1] == (byte)Packet.ETX)       /* 메시지 끝 바이트가 ETX로 끝나면 */
                                {
                                    RxMessage.checksum = RxMessage.data[RxMessage.packetSize - 2];      /* 체크섬 값을 프레임의 체크섬 변수에 저장하고 */
                                    RxMessage.nextStage = (byte)Stage.CHECKSUM;                     /* CHECKSUM 단계로 넘어가기 */
                                }
                                else                                                                /* 메시지 끝 바이트가 ETX가 아니면 */
                                {
                                    RxMessage.nextStage = (byte)Stage.START;                        /* START 단계로 돌아가기 */
                                }
                            }
                            break;

                        }
                    case (byte)Stage.CHECKSUM:
                        {
                            byte checksum = 0;                                       /* 체크섬 계산을 위한 지역 변수 선언 */

                            for (int i = 0; i < RxMessage.packetSize - 2; i++)           /* 1번째 바이트부터 체크섬 앞 바이트까지 XOR */
                            {
                                checksum ^= RxMessage.data[i];
                            }
                            

                            if (RxMessage.checksum == checksum)                     /* 체크섬 값이 정상이면 */
                            {
                                RxMessage.nextStage = (byte)Stage.PARSING;          /* Parsing 단계로 넘어가기 */
                            }
                            else                                                    /* 체크섬 값이 정상이 아니면 */
                            {
                                RxMessage.nextStage = (byte)Stage.START;            /* START 단계로 돌아가기 */
                            }
                            break;
                        }
                    case (byte)Stage.PARSING:
                        {
                            ParsingMessage();
                            RxMessage.nextStage = (byte)Stage.START;
                            break;
                        }
                    case (byte)Stage.END:
                        {
                            break;
                        }
                    default:
                        break;

                }
            }
        }

        //메시지가 정상 일 경우 데이터 처리
        private void ParsingMessage()
        {
            //packet.Deserialize(ref RxMessage.data);
            //packet.stx
            //string strTemp = this.AddBytes(rxBuff.ToList()).Replace("\r", "").Replace("\n", Environment.NewLine);
            //textBox_Debug.AppendText(packet.stx.ToString() + packet.dest.ToString() + packet.src.ToString() + packet.payloadSize.ToString() + packet.checksum.ToString() + Encoding.Default.GetString(packet.payload) + packet.etx.ToString());
            byte[] payloadData = new byte[64];
            Array.Copy(RxMessage.data, 5, payloadData, 0, RxMessage.payloadSize);
            switch (payloadData[0])
             {
                 case 0xC0:
                    float temperature = BitConverter.ToSingle(payloadData, 2); ;
                    float humidity = BitConverter.ToSingle(payloadData, 6); ;

                    textBox_Debug.AppendText("\r\nSRC ID : " + RxMessage.srcID.ToString("X2")  + " Temp: "+ temperature.ToString("F") + "'C, Humi: "+ humidity.ToString("F") + "% \r\n");

                    break;
                 default:
                     break;
             }
             
        }

        bool IsUTF8(byte _byte)
        {
            if ((_byte & 0xE0) == 0xE0) return true;
            return false;

        }

        List<byte> RemainBytes = new List<byte>();
        private String AddBytes(List<byte> _bytes)
        {
            RemainBytes.AddRange(_bytes);

            if (this.RemainBytes.Count >= 2 && IsUTF8(this.RemainBytes[this.RemainBytes.Count - 2]))
            {
                String s = System.Text.Encoding.UTF8.GetString(RemainBytes.ToArray(), 0, this.RemainBytes.Count - 2);
                RemainBytes.RemoveRange(0, this.RemainBytes.Count - 2);
                return s;
            }
            else if (this.RemainBytes.Count >= 1 && IsUTF8(this.RemainBytes[this.RemainBytes.Count - 1]))
            {
                String s = System.Text.Encoding.UTF8.GetString(RemainBytes.ToArray(), 0, this.RemainBytes.Count - 1);
                RemainBytes.RemoveRange(0, this.RemainBytes.Count - 1);
                return s;
            }
            else
            {
                String s = System.Text.Encoding.UTF8.GetString(RemainBytes.ToArray(), 0, this.RemainBytes.Count);
                RemainBytes.Clear();
                return s;
            }
        }

        private void Button_destSend_Click(object sender, EventArgs e)
        {
            /*
            byte[] textbox_data = BitConverter.GetBytes(Convert.ToInt32(textBox_destData.Text, 16));
            byte[] txbuff = {0xA5, 0xA5, (byte)Convert.ToInt32(textBox_destData.Tex };
            txbuff[0] = (byte)Packet.STX;
            txbuff[1] = (byte)Packet.STX;
            //txbuff[2] =;

            Message TxMessage = new Message();
            TxMessage.destID = (byte)Convert.ToInt32(textBox_destData.Text, 16);
                
                , 0xB1, 10, (byte)Convert.ToInt32(textBox_destData.Text, 16), textbox_data[2], textbox_data[1], textbox_data[0], (byte)Packet.CHECKSUM, (byte)Packet.ETX };
                */

            byte[] textbox_data = BitConverter.GetBytes(Convert.ToInt32(textBox_destData.Text, 16));
            
            byte[] txbuff = { 0xA5, 0xA5, (byte)Convert.ToInt32(textBox_destID.Text, 16), 0, 0, 4, (byte)Packet.DATA, (byte)Packet.DATA, (byte)Packet.DATA, (byte)Packet.DATA, (byte)Packet.CHECKSUM, (byte)Packet.ETX };

            Array.Copy(textbox_data, 0, txbuff, 6, 4);
            sendFrame(txbuff);
        }
    }
}
