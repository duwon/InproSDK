using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace SerialTest
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct DataPacket
    {
        [MarshalAs(UnmanagedType.U2)]
        public ushort stx;
        [MarshalAs(UnmanagedType.U1)]
        public byte dest;
        [MarshalAs(UnmanagedType.U1)]
        public byte src;
        [MarshalAs(UnmanagedType.U1)]
        public byte payloadSize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
        public byte[] payload;
        [MarshalAs(UnmanagedType.U1)]
        public byte checksum;
        [MarshalAs(UnmanagedType.U1)]
        public byte etx;

        // Calling this method will return a byte array with the contents
        // of the struct ready to be sent via the tcp socket.
        public byte[] Serialize()
        {
            // allocate a byte array for the struct data
            var buffer = new byte[Marshal.SizeOf(typeof(DataPacket))];

            // Allocate a GCHandle and get the array pointer
            var gch = GCHandle.Alloc(buffer, GCHandleType.Pinned);
            var pBuffer = gch.AddrOfPinnedObject();

            // copy data from struct to array and unpin the gc pointer
            Marshal.StructureToPtr(this, pBuffer, false);
            gch.Free();

            return buffer;
        }

        // this method will deserialize a byte array into the struct.
        public void Deserialize(ref byte[] data)
        {
            var gch = GCHandle.Alloc(data, GCHandleType.Pinned);
            this = (DataPacket)Marshal.PtrToStructure(gch.AddrOfPinnedObject(), typeof(DataPacket));
            gch.Free();
        }
    }
}
