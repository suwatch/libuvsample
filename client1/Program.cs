using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace client1
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                Test(args).Wait();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        static async Task Test(string[] args)
        {
            TcpClient client = new TcpClient();
            await client.ConnectAsync("127.0.0.1", 8000);

            byte[] buf = new byte[4092];

            using (var stream = client.GetStream())
            {
                for (int i = 0; i < 20; ++i)
                {
                    var input = Encoding.UTF8.GetBytes($"({i}){Guid.NewGuid()}");
                    await stream.WriteAsync(input, 0, input.Length);
                    if (i % 2 == 1)
                    {
                        var read = await stream.ReadAsync(buf, 0, buf.Length);
                        Console.WriteLine($"Read = {read} bytes, {Encoding.UTF8.GetString(buf, 0, read)}");
                    }
                }
            }
        }
    }
}
