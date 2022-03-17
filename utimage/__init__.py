import os
import numpy as np
from ctypes import c_ubyte, c_bool
from ctypes import c_int32, c_uint32
from ctypes import c_uint64, c_double
from ctypes import cdll, byref, POINTER


libdir = os.path.abspath(os.path.dirname(__file__))
libpath = os.path.join(libdir, 'libutimage.so')
libut = cdll.LoadLibrary(libpath)

distance_by_dilate_c = libut.distance_by_dilate
color_slice_c = libut.color_slice
resize_mask_c = libut.resize_mask
highlight_mask_border_c = libut.highlight_mask_border
gamma_correct_c = libut.gamma_correct
contrast_stretch_c = libut.contrast_stretch
fluor_filter_c = libut.fluor_filter


def distance_by_dilate(mask1, mask2):
    """
    dilate mask1 until it reaches mask2, return the number of iteration as
    distance between two masks
    mask1 and mask2 are 2D uint8 array of same size

    return: number of iteration as distance
    """
    #print('debug: grow_opened, img.dtype=', img.dtype)

    mask1_c = np.ascontiguousarray(mask1)
    mask2_c = np.ascontiguousarray(mask2)

    #print(img_c.flags)
    c_mask1 = mask1_c.ctypes.data_as(POINTER(c_ubyte))
    c_mask2 = mask2_c.ctypes.data_as(POINTER(c_ubyte))
    c_w = c_int32(mask1.shape[1])
    c_h = c_int32(mask1.shape[0])

    dist = distance_by_dilate_c(c_mask1, c_mask2, c_w, c_h)
    return dist


def color_slice(img, r, g, b, r0):
    """
    run color slice on input image

    img: RGB image
    r, g, b are the center of color
    r0 is the sphere distance limit
    """

    if len(img.shape) != 3 and img.shape[2] != 3:
        print('input must be RGB image')
        exit(1)

    if not img.dtype == np.ubyte:
        min = img.min()
        max = img.max()
        img = (img - min) / (max - min) * 255
        img = img.astype(np.ubyte)

    img_c = np.ascontiguousarray(img)
    c_img = img_c.ctypes.data_as(POINTER(c_ubyte))
    h, w, _ = img.shape
    c_w = c_int32(w)
    c_h = c_int32(h)

    mask = np.zeros((h, w), dtype=np.uint8)
    c_mask = mask.ctypes.data_as(POINTER(c_ubyte))

    #void srm(ubyte *im, ubyte *srmout, int w, int h, int ch, double Q, int mode)
    color_slice_c(c_img, c_mask, c_w, c_h,
                 c_ubyte(r),
                 c_ubyte(g),
                 c_ubyte(b),
                 c_int32(r0))
    return mask


def resize_mask(mask, w, h):
    """
    resize a boolean mask
    Args:
        mask: the mask need to be resized, can be bool or uint8
        w, h: target size

    return: 2D boolean image
    """

    if mask.ndim != 2:
        print('Must be 2D image')
        exit(1)

    if not mask.flags['C_CONTIGUOUS']:
        mask_c = np.ascontiguousarray(mask)
    else:
        mask_c = mask

    c_mask = mask_c.ctypes.data_as(POINTER(c_bool))

    mh, mw = mask.shape
    outim = np.zeros((h, w), dtype=bool)
    c_outim = outim.ctypes.data_as(POINTER(c_bool))

    resize_mask_c(c_mask, mw, mh, c_outim, w, h)
    return outim


def highlight_mask_border(mask):
    """
    modify a mask in-place, highlight mask border
    """

    if mask.ndim != 2:
        print('Must be 2D image')
        exit(1)

    if not mask.flags['C_CONTIGUOUS']:
        mask_c = np.ascontiguousarray(mask)
    else:
        mask_c = mask

    c_mask = mask_c.ctypes.data_as(POINTER(c_bool))

    circum = c_uint32()
    area = c_uint32()
    mh, mw = mask.shape
    highlight_mask_border_c(c_mask, c_int32(mw), c_int32(mh),
                            byref(area), byref(circum))

    return {'mask': mask, 'area': area.value, 'circum': circum.value}


def gamma_correct(im, gamma, c=1.0):
    """
    gamma correct an image
    run color slice on input image

    im: RGB or gray image as narray. If the gray image is a slice from RGB
    image, they should be copied before pass in.

    s = cr^gamma, c and gamma are constant
    """

    if len(im.shape) == 2:
        ch = 1
    elif len(im.shape) == 3:
        ch = im.shape[2]
    else:
        print('Gamma correct: something wrong, image shape is ', im.shape)
        exit(1)

    if not im.dtype == np.ubyte:
        print('convert to ubyte')
        min = im.min()
        max = im.max()
        im = (im - min) / (max - min) * 255
        im = im.astype(np.ubyte)

    if not im.flags['C_CONTIGUOUS']:
        img_c = np.ascontiguousarray(im)
    else:
        img_c = im

    c_img = img_c.ctypes.data_as(POINTER(c_ubyte))
    h, w  = im.shape[0], im.shape[1]
    c_w = c_int32(w)
    c_h = c_int32(h)
    c_ch = c_int32(ch)
    c_c = c_double(c)
    c_gamma = c_double(gamma)

    gamma_correct_c(c_img, c_w, c_h, c_ch, c_gamma, c_c)
    return im


def contrast_stretch(im, low, high):
    """
    linear stretch pixel intensity between low and high

    img: RGB image as narray
    """

    if len(im.shape) != 3 or im.shape[2] != 3:
        print('input must be RGB image')
        exit(1)

    if not im.dtype == np.ubyte:
        print('convert to ubyte')
        min = im.min()
        max = im.max()
        im = (im - min) / (max - min) * 255
        im = im.astype(np.ubyte)

    img_c = np.ascontiguousarray(im)
    c_img = img_c.ctypes.data_as(POINTER(c_ubyte))
    h, w, _ = im.shape
    c_w = c_int32(w)
    c_h = c_int32(h)
    c_low = c_ubyte(low);
    c_high = c_ubyte(high);

    contrast_stretch_c(c_img, c_w, c_h, c_low, c_high)
    return im


def fluor_filter(im, amp_b, color_b, amp_g, color_g, amp_r, color_r):
    """
    img: BGR fluorescence image as narray
    amp_x: amplification in percentage
    color_x: 0xCCCCCC
    """

    if len(im.shape) != 3 or im.shape[2] != 3:
        print('input must be RGB image')
        exit(1)

    if not im.dtype == np.ubyte:
        print('convert to ubyte')
        min = im.min()
        max = im.max()
        im = (im - min) / (max - min) * 255
        im = im.astype(np.ubyte)

    img_c = np.ascontiguousarray(im)
    c_img = img_c.ctypes.data_as(POINTER(c_ubyte))
    h, w, _ = im.shape
    c_w = c_int32(w)
    c_h = c_int32(h)

    c_amp_b = c_int32(amp_b);
    c_amp_g = c_int32(amp_g);
    c_amp_r = c_int32(amp_r);
    c_color_b = c_int32(color_b);
    c_color_g = c_int32(color_g);
    c_color_r = c_int32(color_r);

    fluor_filter_c(c_img, c_w, c_h, c_amp_b, c_color_b, c_amp_g, c_color_g,
                   c_amp_r, c_color_r)
    return im
