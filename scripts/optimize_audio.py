import os
import glob
import subprocess

src_dir = "assets_src/audio"
dst_dir = "assets/audio"

os.makedirs(dst_dir, exist_ok=True)
files = sorted(glob.glob(os.path.join(src_dir, "*.ogg")))

total_orig = 0
total_new = 0

for f in files:
    name = os.path.basename(f)
    out = os.path.join(dst_dir, name)
    orig_sz = os.path.getsize(f)
    total_orig += orig_sz
    
    if name.startswith("call_night"):
        cmd = ["ffmpeg", "-y", "-i", f, "-ac", "1", "-ar", "22050", "-c:a", "libvorbis", "-b:a", "24k", out]
    else:
        cmd = ["ffmpeg", "-y", "-i", f, "-ac", "1", "-c:a", "libvorbis", "-b:a", "48k", out]
        
    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    new_sz = os.path.getsize(out)
    total_new += new_sz
    print(f"  {name}: {orig_sz/1024:.1f} KB -> {new_sz/1024:.1f} KB (-{(1 - new_sz/orig_sz)*100:.1f}%)")

print("Audio re-encoding complete!")
print(f"Original audio size: {total_orig / (1024*1024):.2f} MB")
print(f"Optimized audio size: {total_new / (1024*1024):.2f} MB")
print(f"Saved: {(total_orig - total_new) / (1024*1024):.2f} MB")
