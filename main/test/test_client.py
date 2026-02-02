import asyncio
import struct
from bleak import BleakScanner, BleakClient

# The Service UUID from bluetooth.c
SERVICE_UUID = "59462f12-9543-9999-12c8-58b459a2712d"

# Characteristic UUIDs
STEPS_UUID = "59462f12-9543-9999-12c8-58b459a2712e"
PROGRESS_UUID = "59462f12-9543-9999-12c8-58b459a2712f"
MEDIA_CTRL_UUID = "59462f12-9543-9999-12c8-58b459a27130"
NOTIFY_UUID = "59462f12-9543-9999-12c8-58b459a27131"

def notification_handler(sender, data):
    """Callback for notifications from the ESP32."""
    if sender.uuid == MEDIA_CTRL_UUID:
        cmd = data[0]
        cmd_name = {1: "PLAY/PAUSE", 2: "PREVIOUS", 3: "NEXT"}.get(cmd, f"UNKNOWN({cmd})")
        print(f"[MEDIA] Received command: {cmd_name}")
    elif sender.uuid == NOTIFY_UUID:
        try:
            msg = data.decode('utf-8')
            print(f"[NOTIFY] Message: {msg}")
        except:
            print(f"[NOTIFY] Raw data: {data.hex()}")

async def send_steps(client):
    """Simulate incrementing step count."""
    steps = 1000
    while client.is_connected:
        steps += 10
        print(f"[SENT] Updating steps to: {steps}")
        # Send as little-endian uint32
        data = struct.pack("<I", steps)
        await client.write_gatt_char(STEPS_UUID, data)
        await asyncio.sleep(5)

async def send_music_progress(client):
    """Simulate music progress."""
    progress = 0
    while client.is_connected:
        progress = (progress + 1) % 225
        # print(f"[SENT] Updating progress to: {progress}s")
        data = struct.pack("<I", progress)
        await client.write_gatt_char(PROGRESS_UUID, data)
        await asyncio.sleep(1)

async def main():
    print("Scanning for ESP32 (nimble-test)...")
    device = await BleakScanner.find_device_by_name("nimble-test")
    
    if not device:
        print("Device 'nimble-test' not found.")
        return

    print(f"Found device: {device.name} [{device.address}]")
    
    async with BleakClient(device) as client:
        print(f"Connected: {client.is_connected}")
        
        # Start notifications
        await client.start_notify(MEDIA_CTRL_UUID, notification_handler)
        await client.start_notify(NOTIFY_UUID, notification_handler)
        print("Notifications started. Listening for Media Controls...")

        # Run send loops
        await asyncio.gather(
            send_steps(client),
            send_music_progress(client)
        )

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nExiting...")
