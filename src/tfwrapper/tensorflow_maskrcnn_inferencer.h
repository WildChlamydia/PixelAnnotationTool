#ifndef TENSORFLOW_MASKRCNN_INFERENCER_H
#define TENSORFLOW_MASKRCNN_INFERENCER_H


#include "tensorflow_cnn_inferencer.hpp"
#include "cnpy.h"

#include <QFile>

constexpr int THICKNESS = 2;
constexpr int FONT = cv::FONT_HERSHEY_DUPLEX;
constexpr float FONT_SCALE = 1.1;

inline cv::Mat GetSquareImage( const cv::Mat& img, int target_width )
{
    int width = img.cols,
       height = img.rows;

    cv::Mat square = cv::Mat::zeros( target_width, target_width, img.type() );

    int max_dim = ( width >= height ) ? width : height;
    float scale = ( ( float ) target_width ) / max_dim;
    cv::Rect roi;
    if ( width >= height )
    {
        roi.width = target_width;
        roi.x = 0;
        roi.height = height * scale;
        roi.y = ( target_width - roi.height ) / 2;
    }
    else
    {
        roi.y = 0;
        roi.height = target_width;
        roi.width = width * scale;
        roi.x = ( target_width - roi.width ) / 2;
    }

    cv::resize( img, square( roi ), roi.size() );

    return square;
}

class TensorflowMaskRCNNInferencer : public TensorflowCNNInferencer
{
public:
    TensorflowMaskRCNNInferencer();

    virtual std::string inference(const std::vector<cv::Mat> &imgs);

    // Method to get colored masks only
    std::vector<cv::Mat> getOutputMasks(cv::Size target_size = cv::Size()) const;

    // And method to get bouding boxes
    // Normalized box in interval [0.0; 1.0] as [y0, x0, y1, x1], probability and class
    std::vector<std::vector<std::tuple<cv::Rect2f, float, int> > > getOutputBoundingBoxes() const;

    // Method to get boxes or masks only depends on set niceDraw and drawBoxesOnImage
    std::vector<cv::Mat> getOutputColored(cv::Size target_size = cv::Size(), bool draw_only_boxes = true) const;

    void drawDetectionsOnFrames(std::vector<cv::Mat> outputs) const;


    bool load(const std::string &filename);

    bool loadAnchors(const std::string& npy_filename);
    bool loadMetas(const std::string& npy_filename);


    std::string getAnchorsNpyFilename() const;
    void setAnchorsNpyFilename(const std::string &anchors_npy_filename);

    std::string getMetasNpyFilename() const;
    void setMetasNpyFilename(const std::string &metas_npy_filename);

    float getThresh() const;
    void setThresh(float thresh);

    bool getNiceDraw() const;
    void setNiceDraw(bool nice_draw);

    bool getDrawBoxesOnImage() const;
    void setDrawBoxesOnImage(bool draw_boxes_on_image);

private:

    std::vector<cv::Mat> _last_inferenced_input;

    bool _nice_draw = true;
    bool _draw_boxes_on_image = true;

    int rounded_rectangle( cv::Mat& src, const cv::Rect rect, const cv::Scalar lineColor,
                           const int thickness, const int lineType , const int cornerRadius) const;
    void drawBBOnFrame(cv::Mat output, const cv::Scalar& color, const std::string& objTitle, const cv::Rect& det_rect) const;

    float _thresh = 0.5;

    std::string _anchors_npy_filename;
    std::string _metas_npy_filename;

    std::string _anchor_node_name = "input_anchors";
    std::string _meta_node_name = "input_image_meta";

    std::vector< std::vector< std::vector<float> > > _anchors;
    std::vector< std::vector<float> > _metas;

    std::vector< std::vector< cv::Mat > > _probs_masks;
    std::vector<std::vector<std::tuple<cv::Rect2f, float, int>>> _detections;


};

#endif // TENSORFLOW_MASKRCNN_INFERENCER_H
