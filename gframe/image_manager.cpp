#include "image_manager.h"

namespace ygo {

ImageManager imageManager;

bool ImageManager::Initial() {
	tCover[0] = driver->getTexture("textures/cover.jpg");
	tCover[1] = driver->getTexture("textures/cover2.jpg");
	tUnknown = driver->getTexture("textures/unknown.jpg");
	tAct = driver->getTexture("textures/act.png");
	tAttack = driver->getTexture("textures/attack.png");
	tChain = driver->getTexture("textures/chain.png");
	tNegated = driver->getTexture("textures/negated.png");
	tNumber = driver->getTexture("textures/number.png");
	tLPBar = driver->getTexture("textures/lp.png");
	tLPFrame = driver->getTexture("textures/lpf.png");
	tMask = driver->getTexture("textures/mask.png");
	tEquip = driver->getTexture("textures/equip.png");
	tTarget = driver->getTexture("textures/target.png");
	tLim = driver->getTexture("textures/lim.png");
	tOT = driver->getTexture("textures/ot.png");
	tHand[0] = driver->getTexture("textures/f1.jpg");
	tHand[1] = driver->getTexture("textures/f2.jpg");
	tHand[2] = driver->getTexture("textures/f3.jpg");
	tBackGround = driver->getTexture("textures/bg.jpg");
	tBackGround_menu = driver->getTexture("textures/bg_menu.jpg");
	tBackGround_deck = driver->getTexture("textures/bg_deck.jpg");
	tField = driver->getTexture("textures/field2.png");
	tFieldTransparent = driver->getTexture("textures/field-transparent2.png");
	return true;
}
void ImageManager::SetDevice(irr::IrrlichtDevice* dev) {
	device = dev;
	driver = dev->getVideoDriver();
}
void ImageManager::ClearTexture() {
	for(auto tit = tMap.begin(); tit != tMap.end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	for(auto tit = tThumb.begin(); tit != tThumb.end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	tMap.clear();
	tThumb.clear();
}
void ImageManager::RemoveTexture(int code) {
	auto tit = tMap.find(code);
	if(tit != tMap.end()) {
		if(tit->second)
			driver->removeTexture(tit->second);
		tMap.erase(tit);
	}
}
//https://github.com/minetest/minetest/issues/2419
#define rangelim(d, min, max) ((d) < (min) ? (min) : ((d)>(max)?(max):(d)))
#define SWAP(t, x, y) do { \
	t temp = x;            \
	x = y;                 \
	y = temp;              \
} while (0)
void imageScaleNNAA(video::IImage *src, const core::rect<s32> &srcrect, video::IImage *dest)
{
	double sx, sy, minsx, maxsx, minsy, maxsy, area, ra, ga, ba, aa, pw, ph, pa;
	u32 dy, dx;
	video::SColor pxl;

	// Cache rectsngle boundaries.
	double sox = srcrect.UpperLeftCorner.X * 1.0;
	double soy = srcrect.UpperLeftCorner.Y * 1.0;
	double sw = srcrect.getWidth() * 1.0;
	double sh = srcrect.getHeight() * 1.0;

	// Walk each destination image pixel.
	// Note: loop y around x for better cache locality.
	core::dimension2d<u32> dim = dest->getDimension();
	for (dy = 0; dy < dim.Height; dy++)
		for (dx = 0; dx < dim.Width; dx++) {

			// Calculate floating-point source rectangle bounds.
			// Do some basic clipping, and for mirrored/flipped rects,
			// make sure min/max are in the right order.
			minsx = sox + (dx * sw / dim.Width);
			minsx = rangelim(minsx, 0, sw);
			maxsx = minsx + sw / dim.Width;
			maxsx = rangelim(maxsx, 0, sw);
			if (minsx > maxsx)
				SWAP(double, minsx, maxsx);
			minsy = soy + (dy * sh / dim.Height);
			minsy = rangelim(minsy, 0, sh);
			maxsy = minsy + sh / dim.Height;
			maxsy = rangelim(maxsy, 0, sh);
			if (minsy > maxsy)
				SWAP(double, minsy, maxsy);

			// Total area, and integral of r, g, b values over that area,
			// initialized to zero, to be summed up in next loops.
			area = 0;
			ra = 0;
			ga = 0;
			ba = 0;
			aa = 0;

			// Loop over the integral pixel positions described by those bounds.
			for (sy = floor(minsy); sy < maxsy; sy++)
				for (sx = floor(minsx); sx < maxsx; sx++) {

					// Calculate width, height, then area of dest pixel
					// that's covered by this source pixel.
					pw = 1;
					if (minsx > sx)
						pw += sx - minsx;
					if (maxsx < (sx + 1))
						pw += maxsx - sx - 1;
					ph = 1;
					if (minsy > sy)
						ph += sy - minsy;
					if (maxsy < (sy + 1))
						ph += maxsy - sy - 1;
					pa = pw * ph;

					// Get source pixel and add it to totals, weighted
					// by covered area and alpha.
					pxl = src->getPixel((u32)sx, (u32)sy);
					area += pa;
					ra += pa * pxl.getRed();
					ga += pa * pxl.getGreen();
					ba += pa * pxl.getBlue();
					aa += pa * pxl.getAlpha();
				}

			// Set the destination image pixel to the average color.
			if (area > 0) {
				pxl.setRed(ra / area + 0.5);
				pxl.setGreen(ga / area + 0.5);
				pxl.setBlue(ba / area + 0.5);
				pxl.setAlpha(aa / area + 0.5);
			} else {
				pxl.setRed(0);
				pxl.setGreen(0);
				pxl.setBlue(0);
				pxl.setAlpha(0);
			}
			dest->setPixel(dx, dy, pxl);
		}
}
irr::video::ITexture* ImageManager::GetTexture(int code) {
	if(code == 0)
		return tUnknown;
	auto tit = tMap.find(code);
	if(tit == tMap.end()) {
		char file[256];
		sprintf(file, "expansions/pics/%d.jpg", code);
		irr::video::ITexture* img = driver->getTexture(file);
		if(img == NULL) {
			sprintf(file, "pics/%d.jpg", code);
			img = driver->getTexture(file);
		}
		if(img == NULL) {
			tMap[code] = NULL;
			return GetTextureThumb(code);
		} else {
			video::IImage* srcimg = driver->createImageFromData(img->getColorFormat(),
				img->getSize(), img->lock(), false);
			img->unlock();
			video::IImage *destimg = driver->createImage(img->getColorFormat(),
				core::dimension2d<u32>(177,254));
			imageScaleNNAA(srcimg, core::rect<s32>(0, 0, img->getSize().Width, img->getSize().Height), destimg);
			video::ITexture *scaled = driver->addTexture("233", destimg, NULL);
			tMap[code] = scaled;
			return scaled;
		}
	}
	if(tit->second)
		return tit->second;
	else
		return GetTextureThumb(code);
}
irr::video::ITexture* ImageManager::GetTextureThumb(int code) {
	if(code == 0)
		return tUnknown;
	auto tit = tThumb.find(code);
	if(tit == tThumb.end()) {
		char file[256];
		sprintf(file, "expansions/pics/%d.jpg", code);
		irr::video::ITexture* img = driver->getTexture(file);
		if(img == NULL) {
			sprintf(file, "pics/%d.jpg", code);
			img = driver->getTexture(file);
		}
		if(img == NULL) {
			tThumb[code] = NULL;
			return tUnknown;
		} else {
			video::IImage* srcimg = driver->createImageFromData(img->getColorFormat(),
				img->getSize(), img->lock(), false);
			img->unlock();
			video::IImage *destimg = driver->createImage(img->getColorFormat(),
				core::dimension2d<u32>(44, 64));
			imageScaleNNAA(srcimg, core::rect<s32>(0, 0, img->getSize().Width, img->getSize().Height), destimg);
			video::ITexture *scaled = driver->addTexture("233", destimg, NULL);
			tThumb[code] = scaled;
			return scaled;
			return img;
		}
	}
	if(tit->second)
		return tit->second;
	else
		return tUnknown;
}
irr::video::ITexture* ImageManager::GetTextureField(int code) {
	if(code == 0)
		return NULL;
	auto tit = tFields.find(code);
	if(tit == tFields.end()) {
		char file[256];
		sprintf(file, "expansions/pics/field/%d.png", code);
		irr::video::ITexture* img = driver->getTexture(file);
		if(img == NULL) {
			sprintf(file, "expansions/pics/field/%d.jpg", code);
			img = driver->getTexture(file);
		}
		if(img == NULL) {
			sprintf(file, "pics/field/%d.png", code);
			img = driver->getTexture(file);
		}
		if(img == NULL) {
			sprintf(file, "pics/field/%d.jpg", code);
			img = driver->getTexture(file);
			if(img == NULL) {
				tFields[code] = NULL;
				return NULL;
			} else {
				tFields[code] = img;
				return img;
			}
		} else {
			tFields[code] = img;
			return img;
		}
	}
	if(tit->second)
		return tit->second;
	else
		return NULL;
}
}
