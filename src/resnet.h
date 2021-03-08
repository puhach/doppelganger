#ifndef RESNET_H
#define RESNET_H

#include <optional>
#include <execution>
#include <atomic>
//#include <iostream>     // TEST!

#include <dlib/dnn.h>
#include <dlib/serialize.h>
#include <dlib/image_processing/generic_image.h>	



class ResNet
{
    // The next bit of code defines a ResNet network.  It's basically copied
    // and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
    // layer with loss_metric and made the network somewhat smaller.  Go read the introductory
    // dlib DNN examples to learn what all this stuff means.
    //
    // Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
    // The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
    // essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
    // mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
    // was set to 10000, and the training dataset consisted of about 3 million images instead of
    // 55.  Also, the input layer was locked to images of size 150.
    template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual = dlib::add_prev1<block<N, BN, 1, dlib::tag1<SUBNET>>>;

    template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual_down = dlib::add_prev2<dlib::avg_pool<2, 2, 2, 2, dlib::skip1<dlib::tag2<block<N, BN, 2, dlib::tag1<SUBNET>>>>>>;

    template <int N, template <typename> class BN, int stride, typename SUBNET>
    using block = BN<dlib::con<N, 3, 3, 1, 1, dlib::relu<BN<dlib::con<N, 3, 3, stride, stride, SUBNET>>>>>;

    template <int N, typename SUBNET> using ares = dlib::relu<residual<block, N, dlib::affine, SUBNET>>;
    template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block, N, dlib::affine, SUBNET>>;

    template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
    template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
    template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
    template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
    template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

    using anet_type = dlib::loss_metric<dlib::fc_no_bias<128, dlib::avg_pool_everything<
        alevel0<
        alevel1<
        alevel2<
        alevel3<
        alevel4<
        dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2,
        dlib::input_rgb_image_sized<150>
        >>>>>>>>>>>>;


public:

    using Descriptor = typename anet_type::output_label_type;
    using Input = typename anet_type::input_type;       
    //using PixelType = typename dlib::image_traits<Input>::pixel_type;

    //static constexpr unsigned long inputImageSize = 150;
    static constexpr unsigned long inputSize = 150;     // the size of an input image

    ResNet(const std::string& modelPath) 
    {
        // TODO: consider just dropping the const, esp. when concurrency is disabled
        //dlib::deserialize(modelPath) >> this->net;  // may throw
        dlib::deserialize(modelPath) >> const_cast<anet_type&>(this->net);  // may throw
    }

    ResNet(const ResNet& other) = default;
    ResNet(ResNet&& other) = default;

    ResNet& operator = (const ResNet& other) = default;
    ResNet& operator = (ResNet&& other) = default;

    //std::optional<OutputLabel> operator ()(const Input& input);
    std::optional<Descriptor> operator ()(const Input& input);

    template <class InputIterator, class OutputIterator>
    OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead);

private:

    //const anet_type net;
    anet_type net;
};  // ResNet


//std::optional<ResNet::OutputLabel> ResNet::operator()(const ResNet::Input& input)
std::optional<ResNet::Descriptor> ResNet::operator()(const ResNet::Input& input)
{        
#ifdef PARALLEL_EXECUTION
    // anet_type() is non-const and since we are performing inference in multiple threads, we can't modify the original network.
    auto localNet = this->net;
    return localNet(input);
#else
    // we don't need to copy the original network when concurrency is disabled    
    return this->net(input);
#endif
}   // operator()

template <class InputIterator, class OutputIterator>
OutputIterator ResNet::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
{
   
#ifdef PARALLEL_EXECUTION
    
    // Dlib's batching for face recognition is not really efficient:
    // https://github.com/davisking/dlib/issues/1159

    std::atomic_flag eflag{ false };
    std::exception_ptr eptr;
    outHead = std::transform(std::execution::par, inHead, inTail, outHead,         
        [this, &eflag, &eptr](const Input& input) -> std::optional<ResNet::Descriptor> 
        {
            try
            {

                // Since we are performing inference concurrently and the call operator of anet_type is non-const, we have to make 
                // sure that there is no data race. Using thread_local variables is not an option in this case as they are shared 
                // among all instances of the class, while we want to perform inference by means of the network from this particular 
                // instance. That's why we are making a local copy of the network every time before using it. It may seem inefficient, 
                // but it fact it almost has no impact on the overall processing time. That is for two reasons:        
                // 1) the time to copy the network is small as compared to the inference time 
                // 2) other threads can pick up the slack while we are waiting for a copy

                anet_type faceRecognizer = this->net;
                return faceRecognizer(input);
            }   // try
            catch (...)     // exceptions from other threads are not automatically propagated
            {
                // A read-modify-write operation with this memory order is both an acquire operation and a release operation. 
			    // No memory reads or writes in the current thread can be reordered before or after this store. All writes in 
			    // other threads that release the same atomic variable are visible before the modification and the modification 
			    // is visible in other threads that acquire the same atomic variable.
                if (!eflag.test_and_set(std::memory_order_acq_rel))     // noexcept
                    eptr = std::current_exception();
            }   // catch

            return std::nullopt;
            //return ResNet::OutputLabel();
        });     // transform

    if (eptr)
        std::rethrow_exception(eptr);

#else
    // When parallel execution is disabled (no tbb), use batching
    this->net(inHead, inTail, outHead);
    auto batchSize = inTail - inHead;
    //std::cout << "batch size:" << batchSize << std::endl;
    outHead += batchSize;    
#endif  // !PARALLEL_EXECUTION

    return outHead;
}   // operator ()


//namespace network_traits
//{
//    // TODO: this is probably not needed anymore
//    template <>
//    constexpr inline unsigned long inputImageSize<ResNet> = ResNet::inputSize;
//}



#endif	// RESNET_H
