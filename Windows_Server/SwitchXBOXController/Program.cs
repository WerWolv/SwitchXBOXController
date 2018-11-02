using ScpDriverInterface;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
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
        private static UdpClient udpSocket;

        public static void networking()
        {
            byte[] data = new byte[1024];
            var ipep = new IPEndPoint(IPAddress.Any, 8192);
            udpSocket = new UdpClient(ipep);

            var sender = new IPEndPoint(IPAddress.Any, 0);

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

        static void Main(string[] args)
        {
            Console.WriteLine("|-------------------------------|");
            Console.WriteLine("| Switch XBOX Controller Server |");
            Console.WriteLine("|-------------------------------|");
            Console.WriteLine();
            Console.WriteLine("Running... Please type \"quit\" to close the program.");

            System.Timers.Timer timer;

            bool running = true;

            controller = new X360Controller();
            scpBus = new ScpBus();
            scpBus.PlugIn(1);

            timer = new System.Timers.Timer(1);
            timer.Elapsed += (s, e) => scpBus.Report(1, controller.GetReport());
            timer.AutoReset = true;
            timer.Start();

            networkThread = new Thread(() => networking());

            networkThread.Start();

            while (Console.ReadLine() != "quit") ;

            running = false;
            scpBus.Unplug(1);
            udpSocket.Close();
            networkThread.Join();
        }
    }
}
