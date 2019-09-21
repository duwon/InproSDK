namespace SerialTest
{
    partial class SerialTest
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.sp1 = new System.IO.Ports.SerialPort(this.components);
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label28 = new System.Windows.Forms.Label();
            this.label26 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.comboBox_PortName = new System.Windows.Forms.ComboBox();
            this.button_COMOpen = new System.Windows.Forms.Button();
            this.button_COMClose = new System.Windows.Forms.Button();
            this.label_projectTitle = new System.Windows.Forms.Label();
            this.label_ProjectVersion = new System.Windows.Forms.Label();
            this.textBox_Debug = new System.Windows.Forms.TextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.textBox_RxHex = new System.Windows.Forms.TextBox();
            this.textBox_destID = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBox_destData = new System.Windows.Forms.TextBox();
            this.button_destSend = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.groupBox2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // sp1
            // 
            this.sp1.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(this.sp1_DataReceived);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label28);
            this.groupBox2.Controls.Add(this.label26);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.comboBox_PortName);
            this.groupBox2.Controls.Add(this.button_COMOpen);
            this.groupBox2.Controls.Add(this.button_COMClose);
            this.groupBox2.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.groupBox2.Location = new System.Drawing.Point(12, 54);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.groupBox2.Size = new System.Drawing.Size(256, 110);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Port 속성 설정";
            // 
            // label28
            // 
            this.label28.AutoSize = true;
            this.label28.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label28.Location = new System.Drawing.Point(81, 76);
            this.label28.Name = "label28";
            this.label28.Size = new System.Drawing.Size(72, 15);
            this.label28.TabIndex = 48;
            this.label28.Text = "115200 bps";
            // 
            // label26
            // 
            this.label26.AutoSize = true;
            this.label26.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label26.Location = new System.Drawing.Point(6, 76);
            this.label26.Name = "label26";
            this.label26.Size = new System.Drawing.Size(68, 15);
            this.label26.TabIndex = 47;
            this.label26.Text = "Baud Rate :";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label1.Location = new System.Drawing.Point(6, 35);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(69, 15);
            this.label1.TabIndex = 16;
            this.label1.Text = "Serial Port :";
            // 
            // comboBox_PortName
            // 
            this.comboBox_PortName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBox_PortName.DropDownWidth = 80;
            this.comboBox_PortName.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.comboBox_PortName.FormattingEnabled = true;
            this.comboBox_PortName.Location = new System.Drawing.Point(77, 31);
            this.comboBox_PortName.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.comboBox_PortName.Name = "comboBox_PortName";
            this.comboBox_PortName.Size = new System.Drawing.Size(78, 23);
            this.comboBox_PortName.Sorted = true;
            this.comboBox_PortName.TabIndex = 1;
            this.comboBox_PortName.MouseDown += new System.Windows.Forms.MouseEventHandler(this.comboBox_PortName_MouseDown);
            // 
            // button_COMOpen
            // 
            this.button_COMOpen.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.button_COMOpen.Location = new System.Drawing.Point(164, 30);
            this.button_COMOpen.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.button_COMOpen.Name = "button_COMOpen";
            this.button_COMOpen.Size = new System.Drawing.Size(80, 31);
            this.button_COMOpen.TabIndex = 0;
            this.button_COMOpen.Text = "OPEN";
            this.button_COMOpen.UseVisualStyleBackColor = true;
            this.button_COMOpen.Click += new System.EventHandler(this.button_COMOpen_Click);
            // 
            // button_COMClose
            // 
            this.button_COMClose.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.button_COMClose.Location = new System.Drawing.Point(164, 70);
            this.button_COMClose.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.button_COMClose.Name = "button_COMClose";
            this.button_COMClose.Size = new System.Drawing.Size(80, 31);
            this.button_COMClose.TabIndex = 4;
            this.button_COMClose.Text = "CLOSE";
            this.button_COMClose.UseVisualStyleBackColor = true;
            this.button_COMClose.Click += new System.EventHandler(this.button_COMClose_Click);
            // 
            // label_projectTitle
            // 
            this.label_projectTitle.AutoSize = true;
            this.label_projectTitle.Font = new System.Drawing.Font("맑은 고딕", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label_projectTitle.Location = new System.Drawing.Point(267, 11);
            this.label_projectTitle.Name = "label_projectTitle";
            this.label_projectTitle.Size = new System.Drawing.Size(216, 30);
            this.label_projectTitle.TabIndex = 17;
            this.label_projectTitle.Text = "시리얼 통신 프로그램";
            // 
            // label_ProjectVersion
            // 
            this.label_ProjectVersion.AutoSize = true;
            this.label_ProjectVersion.Font = new System.Drawing.Font("맑은 고딕", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label_ProjectVersion.Location = new System.Drawing.Point(524, 11);
            this.label_ProjectVersion.Name = "label_ProjectVersion";
            this.label_ProjectVersion.Size = new System.Drawing.Size(108, 30);
            this.label_ProjectVersion.TabIndex = 18;
            this.label_ProjectVersion.Text = "Version : ";
            // 
            // textBox_Debug
            // 
            this.textBox_Debug.Dock = System.Windows.Forms.DockStyle.Left;
            this.textBox_Debug.Location = new System.Drawing.Point(0, 0);
            this.textBox_Debug.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.textBox_Debug.Multiline = true;
            this.textBox_Debug.Name = "textBox_Debug";
            this.textBox_Debug.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBox_Debug.Size = new System.Drawing.Size(408, 249);
            this.textBox_Debug.TabIndex = 19;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.textBox_RxHex);
            this.panel1.Controls.Add(this.textBox_Debug);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 171);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(800, 249);
            this.panel1.TabIndex = 20;
            // 
            // textBox_RxHex
            // 
            this.textBox_RxHex.Dock = System.Windows.Forms.DockStyle.Right;
            this.textBox_RxHex.Location = new System.Drawing.Point(414, 0);
            this.textBox_RxHex.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.textBox_RxHex.Multiline = true;
            this.textBox_RxHex.Name = "textBox_RxHex";
            this.textBox_RxHex.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBox_RxHex.Size = new System.Drawing.Size(386, 249);
            this.textBox_RxHex.TabIndex = 20;
            // 
            // textBox_destID
            // 
            this.textBox_destID.Location = new System.Drawing.Point(347, 70);
            this.textBox_destID.Name = "textBox_destID";
            this.textBox_destID.Size = new System.Drawing.Size(34, 23);
            this.textBox_destID.TabIndex = 21;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label2.Location = new System.Drawing.Point(287, 74);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(54, 15);
            this.label2.TabIndex = 22;
            this.label2.Text = "Dest ID :";
            // 
            // textBox_destData
            // 
            this.textBox_destData.Location = new System.Drawing.Point(454, 70);
            this.textBox_destData.Name = "textBox_destData";
            this.textBox_destData.Size = new System.Drawing.Size(235, 23);
            this.textBox_destData.TabIndex = 23;
            // 
            // button_destSend
            // 
            this.button_destSend.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.button_destSend.Location = new System.Drawing.Point(708, 66);
            this.button_destSend.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.button_destSend.Name = "button_destSend";
            this.button_destSend.Size = new System.Drawing.Size(80, 31);
            this.button_destSend.TabIndex = 24;
            this.button_destSend.Text = "SEND";
            this.button_destSend.UseVisualStyleBackColor = true;
            this.button_destSend.Click += new System.EventHandler(this.Button_destSend_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label3.Location = new System.Drawing.Point(403, 74);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(45, 15);
            this.label3.TabIndex = 25;
            this.label3.Text = "DATA :";
            // 
            // SerialTest
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 420);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.button_destSend);
            this.Controls.Add(this.textBox_destData);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.textBox_destID);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.label_ProjectVersion);
            this.Controls.Add(this.label_projectTitle);
            this.Controls.Add(this.groupBox2);
            this.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.Name = "SerialTest";
            this.Text = "시리얼통신";
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.IO.Ports.SerialPort sp1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label28;
        private System.Windows.Forms.Label label26;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBox_PortName;
        private System.Windows.Forms.Button button_COMOpen;
        private System.Windows.Forms.Button button_COMClose;
        private System.Windows.Forms.Label label_projectTitle;
        private System.Windows.Forms.Label label_ProjectVersion;
        private System.Windows.Forms.TextBox textBox_Debug;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.TextBox textBox_RxHex;
        private System.Windows.Forms.TextBox textBox_destID;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBox_destData;
        private System.Windows.Forms.Button button_destSend;
        private System.Windows.Forms.Label label3;
    }
}

