using ScpDriverInterface;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SwitchXBOXController
{
    class Program
    {
        private static ScpBus scpBus;
        private static X360Controller controller;

        private static Thread networkThread;
        private static Thread displayThread;
        private static UdpClient udpSocket;

        static bool running = true;

        [DllImport("Kernel32")]
        private static extern bool SetConsoleCtrlHandler(EventHandler handler, bool add);
        private delegate bool EventHandler();
        static EventHandler _handler;

        public static void networking()
        {
            byte[] data = new byte[1024];
            var ipep = new IPEndPoint(IPAddress.Any, 8192);
            udpSocket = new UdpClient(ipep);

            var sender = new IPEndPoint(IPAddress.Any, 0);

            Console.WriteLine();

            while (true)
            {
                try
                {
                    data = udpSocket.Receive(ref sender);

                    if (data[0] <= 0x0B)
                    {
                        if (data[1] == 0) controller.Buttons &= ~((X360Buttons)(1 << (data[0] - 1)));
                        else controller.Buttons |= ((X360Buttons)(1 << (data[0] - 1)));
                    }
                    else if (data[0] <= 0xF)
                    {
                        if (data[1] == 0) controller.Buttons &= ~((X360Buttons)(1 << (data[0])));
                        else controller.Buttons |= ((X360Buttons)(1 << (data[0])));
                    }
                    else if (data[0] == 0x10)
                    {
                        if (data[1] == 0) controller.LeftTrigger = 0x00;
                        else controller.LeftTrigger = 0xFF;
                    }
                    else if (data[0] == 0x11)
                    {
                        if (data[1] == 0) controller.RightTrigger = 0x00;
                        else controller.RightTrigger = 0xFF;
                    }
                    else if (data[0] == 0x12)
                    {
                        controller.LeftStickX = (short)((data[1] << 8) | data[2]);
                        controller.LeftStickY = (short)((data[3] << 8) | data[4]);
                    }
                    else if (data[0] == 0x13)
                    {
                        controller.RightStickX = (short)((data[1] << 8) | data[2]);
                        controller.RightStickY = (short)((data[3] << 8) | data[4]);
                    }
                }
                catch (SocketException e)
                {
                    break;
                }
            }
        }

        static void displayButtons()
        {
            while(running)
            {
                for (int i = 0; i < 150; i++)
                    Console.Write("\r");
                Console.Write("|{0}|{1}|{2}|{3}|{4}|{5}|{6}|{7}|{8}|{9}|{10}|{11}|{12}|{13}|{14}|{15}|{16}|{17}|{18}|{19}|",
                    (controller.Buttons & X360Buttons.A) > 0                ? "A"           : " ",
                    (controller.Buttons & X360Buttons.B) > 0                ? "B"           : " ",
                    (controller.Buttons & X360Buttons.X) > 0                ? "X"           : " ",
                    (controller.Buttons & X360Buttons.Y) > 0                ? "Y"           : " ",
                    (controller.Buttons & X360Buttons.LeftBumper) > 0       ? "L"           : " ",
                    (controller.Buttons & X360Buttons.RightBumper) > 0      ? "R"           : " ",
                    (controller.LeftTrigger > 0)                            ? "ZL"          : "  ",
                    (controller.RightTrigger > 0)                           ? "ZR"          : "  ",
                    (controller.Buttons & X360Buttons.Up) > 0               ? "UP"          : "  ",
                    (controller.Buttons & X360Buttons.Down) > 0             ? "DOWN"        : "    ",
                    (controller.Buttons & X360Buttons.Left) > 0             ? "LEFT"        : "    ",
                    (controller.Buttons & X360Buttons.Right) > 0            ? "RIGHT"       : "     ",
                    (controller.Buttons & X360Buttons.LeftStick) > 0        ? "LSTICK"      : "      ",
                    (controller.Buttons & X360Buttons.RightStick) > 0       ? "RSTICK"      : "      ",
                    (controller.Buttons & X360Buttons.Start) > 0            ? "START"       :  "     ",
                    (controller.Buttons & X360Buttons.Back) > 0             ? "BACK"        : "    ",
                    "LX " + string.Format("{0:+0.000;-#0.000}", controller.LeftStickX / 32768.0F),
                    "LY " + string.Format("{0:+0.000;-#0.000}", controller.LeftStickY / 32768.0F),
                    "RX " + string.Format("{0:+0.000;-#0.000}", controller.RightStickX / 32768.0F),
                    "RY " + string.Format("{0:+0.000;-#0.000}", controller.RightStickY / 32768.0F));
            }
        }

        private static bool onExitHandler()
        {
            running = false;
            scpBus.Unplug(1);
            udpSocket.Close();
            networkThread.Join();
            displayThread.Join();

            return false;
        }

        static void Main(string[] args)
        {
            Console.WriteLine("|-------------------------------|");
            Console.WriteLine("| Switch XBOX Controller Server |");
            Console.WriteLine("|-------------------------------|");
            Console.WriteLine();

            System.Timers.Timer timer;

            _handler += new EventHandler(onExitHandler);
            SetConsoleCtrlHandler(_handler, true);

            detectSwitch();

            Console.WriteLine("Running...");

            controller = new X360Controller();
            scpBus = new ScpBus();
            scpBus.PlugIn(1);

            timer = new System.Timers.Timer(1);
            timer.Elapsed += (s, e) => scpBus.Report(1, controller.GetReport());
            timer.AutoReset = true;
            timer.Start();

            networkThread = new Thread(() => networking());
            displayThread = new Thread(() => displayButtons());

            networkThread.Start();
            displayThread.Start();

            Console.ReadLine();
        }

        private static void detectSwitch()
        {
            byte[] buffer = new byte[1024];
            var epLocal = new IPEndPoint(IPAddress.Any, 8192);
            EndPoint epRemote = new IPEndPoint(IPAddress.Any, 0);
            var sock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            sock.EnableBroadcast = true;   //new code
            sock.Bind(epLocal);

            while (true)
            {
                sock.ReceiveFrom(buffer, ref epRemote);
                if (Encoding.ASCII.GetString(buffer).Trim().Contains("xbox_switch"))
                {
                    sock.SendTo(Encoding.ASCII.GetBytes("xbox\0"), (IPEndPoint)epRemote);
                    Console.WriteLine($"Connected to: { ((IPEndPoint)epRemote).Address }");
                    break;
                }
            }

            sock.Close();
        }

        private static IPAddress LocalIPAddress()
        {
            if (!System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
            {
                return null;
            }

            IPHostEntry host = Dns.GetHostEntry(Dns.GetHostName());

            return host
                .AddressList
                .FirstOrDefault(ip => ip.AddressFamily == AddressFamily.InterNetwork);
        }

    }
}
