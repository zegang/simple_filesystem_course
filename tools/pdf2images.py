import os
import argparse
from pdf2image import convert_from_path

# poppler/bin的绝对路径
POPPLER_PATH = r"C:\popplerWindows\poppler-25.12.0\Library\bin"

def pdf_to_images(pdf_path, output_dir, dpi=200):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    print(f"正在转换 {pdf_path} ...")
    try:
        images = convert_from_path(
            pdf_path, 
            dpi=dpi, 
            thread_count=4, 
            poppler_path=POPPLER_PATH
        )
        
        for i, image in enumerate(images):
            filename = f"slide{(i+1):02d}.jpg"
            save_path = os.path.join(output_dir, filename)
            image.save(save_path, 'JPEG')
            print(f"已保存: {filename}")
            
        print("\n成功！")
    except Exception as e:
        print(f"转换失败: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="将 PDF 转换为逐页图片")
    parser.add_argument("input", help="输入的 PDF 文件路径")
    parser.add_argument("-o", "--output", default="slides_output", help="图片存储目录 (默认为 slides_output)")
    parser.add_argument("--dpi", type=int, default=200, help="输出分辨率 (默认 200)")

    args = parser.parse_args()
    pdf_to_images(args.input, args.output, args.dpi)