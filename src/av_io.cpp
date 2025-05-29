#include <filesystem>
#include <ff.hpp>
#include <Magick++.h>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
}

bool ff::convert_to_webp(const std::string& input, const std::string& output) {
    try {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Converting " + input + " to WEBP.\n");
#endif
        Magick::InitializeMagick(nullptr);
        Magick::Image image;
        image.read(input);
        image.magick("WEBP");
        image.write(output);

#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Conversion successful.\n");
#endif

        return std::filesystem::is_regular_file(output);
    } catch (const std::exception& e) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::error, "Failed to convert to WEBP: " + std::string(e.what()) + "\n");
#endif
        return false;
    }
}

// TODO: Function sucks because it uses system() instead of the FFmpeg API.
// Sadly, I'm too dumb to figure out how to use the FFmpeg API for this and after hours
// of trying, I'm just going to use system() for now.
bool ff::convert_to_webm(const std::string& input, const std::string& output) {
#ifdef FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Converting " + input + " to WEBM.\n");
#endif

    std::string command = "ffmpeg -i " + input + " -c:v libvpx-vp9 -b:v 1M -c:a libopus -b:a 192k -f webm -loglevel quiet " + output;
    int ret = std::system(command.c_str());

#ifdef FF_DEBUG
    if (ret != 0) {
        logger.write_to_log(limhamn::logger::type::error, "Failed to convert to WEBM.\n");
    } else {
        logger.write_to_log(limhamn::logger::type::notice, "Conversion successful.\n");
    }
#endif

    return ret == 0;
}

bool ff::validate_video(const std::string& path) {
    AVFormatContext* fc = nullptr;

#ifdef FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Validating video: " + path + "\n");
#endif

    if (avformat_open_input(&fc, path.c_str(), nullptr, nullptr) != 0) {
        return false;
    }

    if (avformat_find_stream_info(fc, nullptr) < 0) {
        avformat_close_input(&fc);
        return false;
    }

    for (unsigned int i = 0; i < fc->nb_streams; i++) {
        if (fc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            avformat_close_input(&fc);
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Video stream found.\n");
#endif
            return true;
        }
    }

    avformat_close_input(&fc);

#ifdef FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "No video stream found.\n");
#endif

    return false;
}

bool ff::validate_image(const std::string& path) {
    try {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::notice, "Validating image: " + path + "\n");
#endif
        Magick::InitializeMagick(nullptr);
        Magick::Image image;
        image.read(path);

#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::notice, "Image validated.\n");
#endif
        return true;
    } catch (const Magick::Exception& e) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::error, "Failed to validate image: " + std::string(e.what()) + "\n");
#endif
        return false;
    }
}
