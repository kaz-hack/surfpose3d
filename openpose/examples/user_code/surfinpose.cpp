// ------------------------- OpenPose Library Tutorial - Pose - Example 1 - Extract from Image -------------------------
// This first example shows the user how to:
    // 1. Load an image (`filestream` module)
    // 2. Extract the pose of that image (`pose` module)
    // 3. Render the pose on a resized copy of the input image (`pose` module)
    // 4. Display the rendered pose (`gui` module)
// In addition to the previous OpenPose modules, we also need to use:
    // 1. `core` module: for the Array<float> class that the `pose` module needs
    // 2. `utilities` module: for the error & logging functions, i.e. op::error & op::log respectively

// 3rdparty dependencies
#include <gflags/gflags.h> // DEFINE_bool, DEFINE_int32, DEFINE_int64, DEFINE_uint64, DEFINE_double, DEFINE_string
#include <glog/logging.h> // google::InitGoogleLogging
// OpenPose dependencies
#include <openpose/core/headers.hpp>
#include <openpose/filestream/headers.hpp>
#include <openpose/gui/headers.hpp>
#include <openpose/pose/headers.hpp>
#include <openpose/utilities/headers.hpp>

// See all the available parameter options withe the `--help` flag. E.g. `./build/examples/openpose/openpose.bin --help`.
// Note: This command will show you flags for other unnecessary 3rdparty files. Check only the flags for the OpenPose
// executable. E.g. for `openpose.bin`, look for `Flags from examples/openpose/openpose.cpp:`.
// Debugging

DEFINE_int32(logging_level,             3,              "The logging level. Integer in the range [0, 255]. 0 will output any log() message, while"
                                                        " 255 will not output any. Current OpenPose library messages are in the range 0-4: 1 for"
                                                        " low priority messages and 4 for important ones.");
// Producer
DEFINE_string(image_path,               "examples/media/COCO_val2014_000000000192.jpg",     "Process the desired image.");
// OpenPose
DEFINE_string(model_pose,               "COCO",         "Model to be used. E.g. `COCO` (18 keypoints), `MPI` (15 keypoints, ~10% faster), "
                                                        "`MPI_4_layers` (15 keypoints, even faster but less accurate).");
DEFINE_string(model_folder,             "models/",      "Folder path (absolute or relative) where the models (pose, face, ...) are located.");
DEFINE_string(net_resolution,           "656x368",      "Multiples of 16. If it is increased, the accuracy potentially increases. If it is decreased,"
                                                        " the speed increases. For maximum speed-accuracy balance, it should keep the closest aspect"
                                                        " ratio possible to the images or videos to be processed. E.g. the default `656x368` is"
                                                        " optimal for 16:9 videos, e.g. full HD (1980x1080) and HD (1280x720) videos.");
DEFINE_string(resolution,               "1280x720",     "The image resolution (display and output). Use \"-1x-1\" to force the program to use the"
                                                        " default images resolution.");
DEFINE_int32(num_gpu_start,             0,              "GPU device start number.");
DEFINE_double(scale_gap,                0.3,            "Scale gap between scales. No effect unless scale_number > 1. Initial scale is always 1."
                                                        " If you want to change the initial scale, you actually want to multiply the"
                                                        " `net_resolution` by your desired initial scale.");
DEFINE_int32(scale_number,              1,              "Number of scales to average.");
// OpenPose Rendering
DEFINE_bool(disable_blending,           false,          "If blending is enabled, it will merge the results with the original frame. If disabled, it"
                                                        " will only display the results on a black background.");
DEFINE_double(render_threshold,         0.05,           "Only estimated keypoints whose score confidences are higher than this threshold will be"
                                                        " rendered. Generally, a high threshold (> 0.5) will only render very clear body parts;"
                                                        " while small thresholds (~0.1) will also output guessed and occluded keypoints, but also"
                                                        " more false positives (i.e. wrong detections).");
DEFINE_double(alpha_pose,               0.6,            "Blending factor (range 0-1) for the body part rendering. 1 will show it completely, 0 will"
                                                        " hide it. Only valid for GPU rendering.");

using namespace std;
using namespace cv;

//openposeを実行して骨格を描画したMatを返す
cv::Mat execOp(cv::Mat inputImage,
                op::CvMatToOpInput *cvMatToOpInput,
                op::CvMatToOpOutput *cvMatToOpOutput,
                op::PoseExtractorCaffe *poseExtractorCaffe,
                op::PoseRenderer *poseRenderer,
                op::OpOutputToCvMat *opOutputToCvMat)
{
    // ------------------------- POSE ESTIMATION AND RENDERING -------------------------
    // Step 1 - Read and load image, error if empty (possibly wrong path)
    //cv::Mat inputImage = op::loadImage(FLAGS_image_path, CV_LOAD_IMAGE_COLOR); // Alternative: cv::imread(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
    //cv::Mat inputImage = cv::imread(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
    if(inputImage.empty())
        op::error("Could not open or find the image: " + FLAGS_image_path, __LINE__, __FUNCTION__, __FILE__);
    // Step 2 - Format input image to OpenPose input and output formats
    op::Array<float> netInputArray;
    std::vector<float> scaleRatios;
    std::tie(netInputArray, scaleRatios) = cvMatToOpInput->format(inputImage);
    double scaleInputToOutput;
    op::Array<float> outputArray;
    std::tie(scaleInputToOutput, outputArray) = cvMatToOpOutput->format(inputImage);
    // Step 3 - Estimate poseKeypoints
    poseExtractorCaffe->forwardPass(netInputArray, {inputImage.cols, inputImage.rows}, scaleRatios);
    const auto poseKeypoints = poseExtractorCaffe->getPoseKeypoints();
    // Step 4 - Render poseKeypoints
    poseRenderer->renderPose(outputArray, poseKeypoints);
    // Step 5 - OpenPose output format to cv::Mat
    cv::Mat outputImage = opOutputToCvMat->formatToCvMat(outputArray);
    
    std::vector<cv::Point2f> bodyPoints2D;
    for(int i = 0; i<18 ;i++){
        cv::Point2f _bodyPoint(poseKeypoints[3*i], poseKeypoints[3*i+1]);
        bodyPoints2D.push_back(_bodyPoint);
    }
    for(int i = 0; i<18 ;i++){
        cv::circle(outputImage, bodyPoints2D[i], 3, cv::Scalar(0,0,200), -1);
        //std::cout << "bodyPoints2DVec[" <<  i << "] : " << bodyPoints2DVec[i] << std::endl;
    }
    // ------------------------- SHOWING RESULT AND CLOSING -------------------------
    // Step 1 - Show results
    //frameDisplayer->displayFrameoutputImage, 0); // Alternative: cv::imshow(outputImage) + cv::waitKey(0)
    cv::imshow("outputImage",outputImage);
    return outputImage;
    //cv::waitKey(0);
    // Step 2 - Logging information message
    //op::log("Example 1 successfully finished.", op::Priority::High);
}

// openposeを実行して関節座標をベクトルで返す
std::vector<cv::Point2f> getEstimated2DPoseVec(cv::Mat inputImage,
                                               op::CvMatToOpInput *cvMatToOpInput,
                                               op::CvMatToOpOutput *cvMatToOpOutput,
                                               op::PoseExtractorCaffe *poseExtractorCaffe)
{
  // ------------------------- POSE ESTIMATION AND RENDERING -------------------------
    // Step 1 - Read and load image, error if empty (possibly wrong path)
    //cv::Mat inputImage = op::loadImage(FLAGS_image_path, CV_LOAD_IMAGE_COLOR); // Alternative: cv::imread(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
    //cv::Mat inputImage = cv::imread(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
    if(inputImage.empty())
        op::error("Could not open or find the image: " + FLAGS_image_path, __LINE__, __FUNCTION__, __FILE__);
    // Step 2 - Format input image to OpenPose input and output formats
    op::Array<float> netInputArray;
    std::vector<float> scaleRatios;
    std::tie(netInputArray, scaleRatios) = cvMatToOpInput->format(inputImage);
    double scaleInputToOutput;
    op::Array<float> outputArray;
    std::tie(scaleInputToOutput, outputArray) = cvMatToOpOutput->format(inputImage);
    // Step 3 - Estimate poseKeypoints
    poseExtractorCaffe->forwardPass(netInputArray, {inputImage.cols, inputImage.rows}, scaleRatios);
    const auto poseKeypoints = poseExtractorCaffe->getPoseKeypoints();

    std::vector<cv::Point2f> bodyPoints2D;
    for(int i = 0; i<18 ;i++){
        cv::Point2f _bodyPoint(poseKeypoints[3*i], poseKeypoints[3*i+1]);
        bodyPoints2D.push_back(_bodyPoint);
        //cout<<bodyPoints2D[i]<<endl;
    }
    return bodyPoints2D;
}

// openposeを実行して関節座標をMatで返す
cv::Mat getEstimated2DPoseMat(cv::Mat inputImage,
                               op::CvMatToOpInput *cvMatToOpInput,
                               op::CvMatToOpOutput *cvMatToOpOutput,
                               op::PoseExtractorCaffe *poseExtractorCaffe)
{
    // Step 1 - Read and load image, error if empty (possibly wrong path)
    //cv::Mat inputImage = op::loadImage(FLAGS_image_path, CV_LOAD_IMAGE_COLOR); // Alternative: cv::imread(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
    //cv::Mat inputImage = cv::imread(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
    if(inputImage.empty())
        op::error("Could not open or find the image: " + FLAGS_image_path, __LINE__, __FUNCTION__, __FILE__);
    // Step 2 - Format input image to OpenPose input and output formats
    op::Array<float> netInputArray;
    std::vector<float> scaleRatios;
    std::tie(netInputArray, scaleRatios) = cvMatToOpInput->format(inputImage);
    double scaleInputToOutput;
    op::Array<float> outputArray;
    std::tie(scaleInputToOutput, outputArray) = cvMatToOpOutput->format(inputImage);
    // Step 3 - Estimate poseKeypoints
    poseExtractorCaffe->forwardPass(netInputArray, {inputImage.cols, inputImage.rows}, scaleRatios);
    const auto poseKeypoints = poseExtractorCaffe->getPoseKeypoints();
    cv::Mat bodyPoints2D = cv::Mat::zeros(2,18,CV_32FC1);
    for (int i = 0; i<18; i++){
        bodyPoints2D.at<float>(0,i) = poseKeypoints[3*i];
        bodyPoints2D.at<float>(1,i) = poseKeypoints[3*i+1];
        //cout << "poseKeyPoints[" << i << "] : " << poseKeyPoints[3*i] << "," << poseKeyPoints[3*i+1] << std::endl;
    }
    return bodyPoints2D;
}

int main(int argc, char *argv[])
{
    cv::Mat colorImage = cv::imread("examples/media/goprosurfin.jpg");

    // Initializing google logging (Caffe uses it for logging)
    google::InitGoogleLogging("openPoseTutorialPose1");

    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Running openPoseTutorialPose1
    //initializeOp();

    // Initializing openpose
    op::log("OpenPose Library Tutorial - Example 1.", op::Priority::High);
    // ------------------------- INITIALIZATION -------------------------
    // Step 1 - Set logging level
        // - 0 will output all the logging messages
        // - 255 will output nothing
    op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.", __LINE__, __FUNCTION__, __FILE__);
    op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
    op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
    
    // Step 2 - Read Google flags (user defined configuration)
    // outputSize
    //規定値から変更。入力画像のサイズを読み取り、その画像における関節座標を取得する
    stringstream ss;
    ss<<colorImage.cols<<"x"<<colorImage.rows<<endl;
    const auto outputSize = op::flagsToPoint(ss.str());
    // netInputSize
    const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "656x368");
    // netOutputSize
    const auto netOutputSize = netInputSize;
    // poseModel
    const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);
    
    // Check no contradictory flags enabled
    if (FLAGS_alpha_pose < 0. || FLAGS_alpha_pose > 1.)
        op::error("Alpha value for blending must be in the range [0,1].", __LINE__, __FUNCTION__, __FILE__);
    if (FLAGS_scale_gap <= 0. && FLAGS_scale_number > 1)
        op::error("Incompatible flag configuration: scale_gap must be greater than 0 or scale_number = 1.", __LINE__, __FUNCTION__, __FILE__);
    // Logging
    op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
    // Step 3 - Initialize all required classes
    op::CvMatToOpInput cvMatToOpInput{netInputSize, FLAGS_scale_number, (float)FLAGS_scale_gap};
    op::CvMatToOpOutput cvMatToOpOutput{outputSize};
    op::PoseExtractorCaffe poseExtractorCaffe{netInputSize, netOutputSize, outputSize, FLAGS_scale_number, poseModel,
                                              FLAGS_model_folder, FLAGS_num_gpu_start};
    op::PoseRenderer poseRenderer{netOutputSize, outputSize, poseModel, nullptr, (float)FLAGS_render_threshold,
                                  !FLAGS_disable_blending, (float)FLAGS_alpha_pose};
    op::OpOutputToCvMat opOutputToCvMat{outputSize};
    const op::Point<int> windowedSize = outputSize;
    op::FrameDisplayer frameDisplayer{windowedSize, "OpenPose Tutorial - Example 1"};
    // Step 4 - Initialize resources on desired thread (in this case single thread, i.e. we init resources here)
    poseExtractorCaffe.initializationOnThread();
    poseRenderer.initializationOnThread();

    //ディレクトリにアクセス
    //Ex.("media/"")

    // 動画のフレームを抜き出しcolorImage
    // 各フレームに対しopenposeを実行して座標を描画してMatの形で獲得
    
    /*
    // openposeを実行してcv::Matを返す
    cv::Mat outputImg = execOp(colorImage,
                                &cvMatToOpInput,
                                &cvMatToOpOutput,
                                &poseExtractorCaffe,
                                &poseRenderer,
                                &opOutputToCvMat);
    */
    // openposeを実行して関節座標をベクトルで返す
    std::vector<cv::Point2f> bodyPoints2DVec = getEstimated2DPoseVec(colorImage,
                                                                      &cvMatToOpInput,
                                                                      &cvMatToOpOutput,
                                                                      &poseExtractorCaffe);
    /*
    // openposeを実行して関節座標をMatで返す
    cv::Mat bodyPoints2DMat = getEstimated2DPoseMat(colorImage,
                                                      &cvMatToOpInput,
                                                      &cvMatToOpOutput,
                                                      &poseExtractorCaffe);
    
    */
    for(int i = 0; i<18 ;i++){
        cout<<"point["<<i<<"]"<<bodyPoints2DVec[i]<<endl;
        cv::circle(colorImage, bodyPoints2DVec[i], 3, cv::Scalar(0,0,200), -1);
        //std::cout << "bodyPoints2DVec[" <<  i << "] : " << bodyPoints2DVec[i] << std::endl;
    }

    cv::imshow("outputImage",colorImage);
    
    cv::waitKey(0);

    if(int key = waitKey(113)){
        return 1;
    }

    return 0;
}