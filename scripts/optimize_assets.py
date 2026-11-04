import os
import glob
from PIL import Image

src_dir = "assets_src/images/cams"
dst_dir = "assets/images/cams"

os.makedirs(dst_dir, exist_ok=True)
pngs = sorted(glob.glob(os.path.join(src_dir, "**/*.png"), recursive=True))

print(f"Optimizing {len(pngs)} camera feed assets...")
for path in pngs:
    rel = os.path.relpath(path, src_dir)
    out_path = os.path.join(dst_dir, rel)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)

    im = Image.open(path)
    
    # Don't downscale or grayscale the UI map
    if rel == "map.png":
        im.save(out_path, "PNG", optimize=True, compress_level=9)
        continue

    # Camera feeds: downscale to ~640x360 (proportional)
    w, h = im.size
    if w == 1600 and h == 720:
        target_size = (640, 288) # Exact 2.5x downscale preserving aspect
    elif w == 1280 and h == 720:
        target_size = (640, 360) # Exact 2x downscale preserving aspect
    else:
        # scale proportionally so max dimension is around 640
        scale = 640.0 / w
        target_size = (640, int(h * scale))

    im_resized = im.resize(target_size, Image.Resampling.LANCZOS)
    
    # Convert to 8-bit Grayscale (L mode)
    im_gray = im_resized.convert("L")
    im_gray.save(out_path, "PNG", optimize=True, compress_level=9)
    print(f"  {rel}: {im.size} -> {im_gray.size} (Grayscale)")

print("Camera asset optimization complete!")
