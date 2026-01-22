#!/usr/bin/env python3
"""
Rendering quality metrics for MCPS renderer
Computes MSE, PSNR, SSIM, and optionally LPIPS between rendered and reference images
"""

import numpy as np
from PIL import Image
import argparse

def load_ppm(filename):
    """Load PPM image"""
    img = Image.open(filename)
    return np.array(img) / 255.0

def mse(img1, img2):
    """Mean Squared Error"""
    return np.mean((img1 - img2) ** 2)

def psnr(img1, img2):
    """Peak Signal-to-Noise Ratio"""
    mse_val = mse(img1, img2)
    if mse_val == 0:
        return float('inf')
    return 20 * np.log10(1.0 / np.sqrt(mse_val))

def ssim(img1, img2, window_size=11):
    """Structural Similarity Index (simplified)"""
    # Simplified SSIM implementation
    mu1 = np.mean(img1)
    mu2 = np.mean(img2)
    sigma1 = np.var(img1)
    sigma2 = np.var(img2)
    sigma12 = np.mean((img1 - mu1) * (img2 - mu2))
    
    c1 = (0.01) ** 2
    c2 = (0.03) ** 2
    
    ssim_val = ((2 * mu1 * mu2 + c1) * (2 * sigma12 + c2)) / \
               ((mu1 ** 2 + mu2 ** 2 + c1) * (sigma1 + sigma2 + c2))
    
    return ssim_val

def main():
    parser = argparse.ArgumentParser(description='Compute rendering quality metrics')
    parser.add_argument('rendered', help='Rendered image (PPM)')
    parser.add_argument('reference', help='Reference image (PPM)')
    parser.add_argument('--lpips', action='store_true', help='Compute LPIPS (requires PyTorch)')
    
    args = parser.parse_args()
    
    print("Loading images...")
    img1 = load_ppm(args.rendered)
    img2 = load_ppm(args.reference)
    
    print(f"\nImage shapes: {img1.shape}, {img2.shape}")
    
    if img1.shape != img2.shape:
        print("ERROR: Image dimensions don't match!")
        return
    
    print("\nComputing metrics...")
    mse_val = mse(img1, img2)
    psnr_val = psnr(img1, img2)
    ssim_val = ssim(img1, img2)
    
    print(f"MSE:  {mse_val:.6f}")
    print(f"PSNR: {psnr_val:.2f} dB")
    print(f"SSIM: {ssim_val:.4f}")
    
    if args.lpips:
        try:
            import torch
            import lpips
            
            print("\nComputing LPIPS...")
            loss_fn = lpips.LPIPS(net='alex')
            
            # Convert to torch tensors
            img1_torch = torch.from_numpy(img1).permute(2, 0, 1).unsqueeze(0).float()
            img2_torch = torch.from_numpy(img2).permute(2, 0, 1).unsqueeze(0).float()
            
            lpips_val = loss_fn(img1_torch, img2_torch).item()
            print(f"LPIPS: {lpips_val:.4f}")
        except ImportError:
            print("\nLPIPS requires PyTorch and lpips package")
            print("Install with: pip install torch lpips")

if __name__ == '__main__':
    main()
