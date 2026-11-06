#!/usr/bin/env python3
import os
import sys
import struct
import io
from PIL import Image

def pack_assets(source_dirs, output_path):
    bundle = bytearray()
    bundle.extend(b'FNAF')
    bundle.extend(struct.pack('<I', 1)) # Version 1

    files_to_pack = []
    for sdir in source_dirs:
        if not os.path.exists(sdir):
            continue
        sdir_abs = os.path.abspath(sdir)
        base_dir = os.path.dirname(sdir_abs)
        for root, _, files in os.walk(sdir_abs):
            for f in sorted(files):
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, base_dir).replace('\\', '/').lstrip('/')
                files_to_pack.append((rel_path, full_path))

    bundle.extend(struct.pack('<I', len(files_to_pack)))
    for rel_path, full_path in files_to_pack:
        rel_bytes = rel_path.encode('utf-8')
        data = None

        # Optimize opaque images (jumpscares, camera feeds, office, menu freddy frames) as high-quality JPEG (Q=85).
        # Raylib uses stb_image with SUPPORT_FILEFORMAT_JPG which loads JPEG transparently.
        # Images with alpha transparency (e.g. map.png, UI sprites, scanlines) stay as pure PNG.
        if rel_path.endswith('.png'):
            try:
                with Image.open(full_path) as img:
                    has_alpha = False
                    if img.mode in ('RGBA', 'LA'):
                        alpha_channel = img.split()[-1]
                        min_alpha, _ = alpha_channel.getextrema()
                        if min_alpha < 255:
                            has_alpha = True
                    elif img.mode == 'P' and 'transparency' in img.info:
                        has_alpha = True

                    if not has_alpha and img.size[0] > 64 and img.size[1] > 64:
                        buf = io.BytesIO()
                        img.convert('RGB').save(buf, format='JPEG', quality=85, optimize=True)
                        data = buf.getvalue()
            except Exception:
                data = None

        if data is None:
            with open(full_path, 'rb') as f:
                data = f.read()

        bundle.extend(struct.pack('<H', len(rel_bytes)))
        bundle.extend(rel_bytes)
        bundle.extend(struct.pack('<I', len(data)))
        bundle.extend(data)

    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    with open(output_path, 'wb') as f:
        f.write(bundle)

    print(f"[Assets Pack] Packed {len(files_to_pack)} files ({len(bundle)/1024/1024:.2f} MB) -> {output_path}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: pack_assets.py <output_bundle> <dir1> [dir2 ...]")
        sys.exit(1)
    pack_assets(sys.argv[2:], sys.argv[1])
