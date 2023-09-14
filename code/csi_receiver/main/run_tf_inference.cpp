#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "run_tf_inference.h"
#include "model.h"
#define MODEL_STEPS 64

namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// allocate memory for tensorflow (statically)
const int tensor_arena_size = 40 * 1024;
uint8_t tensor_arena[tensor_arena_size];
}


extern "C" void model_setup(){
    // load the model and check that its schema is compatible with the version we are using
    model = ::tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        printf("Model provided is schema version %d not equal to supported version %d\n", model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // instantiate the operations resolver which is used to register and access the operations used by the model
    static tflite::MicroMutableOpResolver<9> resolver;
    if (resolver.AddFullyConnected() != kTfLiteOk) {
        return;
    }
    if (resolver.AddSoftmax() != kTfLiteOk) {
        return;
    }
    // things for CNN
    if (resolver.AddPad() != kTfLiteOk) {
        return;
    }
    if (resolver.AddExpandDims() != kTfLiteOk) {
        return;
    }
    if (resolver.AddConv2D() != kTfLiteOk) {
        return;
    }
    if (resolver.AddReshape() != kTfLiteOk) {
        return;
    }
    if (resolver.AddMaxPool2D() != kTfLiteOk) {
        return;
    }
    // things for LSTM
    if (resolver.AddUnidirectionalSequenceLSTM() != kTfLiteOk) {
        return;
    }
    if (resolver.AddStridedSlice() != kTfLiteOk) {
        return;
    }
    


    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, tensor_arena_size);
    interpreter = &static_interpreter;

    // allocate memory from the arena for the model tensors
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        printf("AllocateTensors() failed");
        return;
    }

    // Obtain a pointer to the model's input and output tensor
    input = interpreter->input(0);
    output = interpreter->output(0);
    
    printf("input type %d, input dims size %d, dims.data[0] %d, dims.data[1] %d\n", input->type, input->dims->size, input->dims->data[0], input->dims->data[1]);
}

extern "C" int run_inference(float* input_values){
    // set the input values
    for (int i = 0; i < MODEL_STEPS*42; i++) {
        input->data.f[i] = input_values[i];
    }

    // run the model
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        //printf("Invoke failed\n");
    }

    TfLiteTensor* output = interpreter->output(0);
    
    // return the result -> max index
    float current_max = -1;
    int max_index = 0;
    for(int i=0; i<6; i++){
        if(output->data.f[i] > current_max){
            current_max = output->data.f[i];
            max_index = i;
        }
    }
    return max_index;
}