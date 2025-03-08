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
        Magick::InitializeMagick(nullptr);
        Magick::Image image;
        image.read(input);
        image.magick("WEBP");
        image.write(output);

        return std::filesystem::is_regular_file(output);
    } catch (const std::exception& e) {
        return false;
    }
}

// TODO: Function sucks because it uses system() instead of the FFmpeg API.
// Sadly, I'm too dumb to figure out how to use the FFmpeg API for this and after hours
// of trying, I'm just going to use system() for now.
bool ff::convert_to_webm(const std::string& input, const std::string& output) {
    std::string command = "ffmpeg -i " + input + " -c:v libvpx -b:v 1M -c:a libvorbis -b:a 192k -f webm -loglevel quiet " + output;
    return std::system(command.c_str()) == 0;
}

bool ff::validate_video(const std::string& path) {
    AVFormatContext* fc = nullptr;

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
            return true;
        }
    }

    avformat_close_input(&fc);
    return false;
}

bool ff::validate_image(const std::string& path) {
    try {
        Magick::InitializeMagick(nullptr);
        Magick::Image image;
        image.read(path);
        return true;
    } catch (const Magick::Exception& e) {
        return false;
    }
}
