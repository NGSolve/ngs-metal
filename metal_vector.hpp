#ifndef FILE_METAL_VECTOR_HPP
#define FILE_METAL_VECTOR_HPP

#include <basevector.hpp>

namespace ngsmetal
{
  using namespace ngla;

  MTL::Function* saxpyFunc = nullptr;
  MTL::Function* scaleFunc = nullptr;  
   
  void InitVectorKernels()
  {
    // 3. Embedded Metal Shading Language (MSL) Kernel Source
    const char* shaderSource = R"(
        #include <metal_stdlib>
        using namespace metal;

        kernel void saxpy(device const float* x [[buffer(0)]],
                          device float* y       [[buffer(1)]],
                          constant float& a     [[buffer(2)]],
                          uint id                [[thread_position_in_grid]]) {
            y[id] = a * x[id] + y[id];
        }

        kernel void scale(device float* x       [[buffer(0)]],
                          constant float& a     [[buffer(1)]],
                          uint id                [[thread_position_in_grid]]) {
            x[id] *= a;
        }
    )";

    // 4. Compile MSL source string into compute pipeline state
    NS::Error* error = nullptr;
    NS::String* mslCode = NS::String::string(shaderSource, NS::UTF8StringEncoding);
    MTL::Library* library = device->newLibrary(mslCode, nullptr, &error);

    if (!library) {
      throw Exception("Shader compile error: " 
                      +string(error->localizedDescription()->utf8String()));
    }

    {
      NS::String* funcName = NS::String::string("saxpy", NS::UTF8StringEncoding);
      cout << "set saxpy function" << endl;
      saxpyFunc = library->newFunction(funcName);
      // MTL::ComputePipelineState* pipelineState = device->newComputePipelineState(saxpyFunc, &error);
    }
    {
      NS::String* funcName = NS::String::string("scale", NS::UTF8StringEncoding);
      cout << "set scale function" << endl;
      scaleFunc = library->newFunction(funcName);
    }
  }
  
  class MetalVector : public S_BaseVector<float>
  {
    MTL::Buffer* buffer;
    
  public:
    MetalVector (size_t s)
    {
      this -> size = s;
      buffer = device->newBuffer(s*sizeof(float), MTL::ResourceStorageModeShared);

      // just to set something
      float * values = (float*)buffer->contents();
      for (size_t i = 0; i < size; i++)
        values[i] = float(i);
    }


    virtual BaseVector & Scale (double scal) override
    {
      /*
      float * values = (float*)buffer->contents();      
      for (size_t i = 0; i < size; i++)
        values[i] *= scal;
      */
      
      NS::Error* error = nullptr;      
      MTL::ComputePipelineState* pipelineState = device->newComputePipelineState(scaleFunc, &error);
      
      // 6. Encode and submit compute commands
      MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
      MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();

      encoder->setComputePipelineState(pipelineState);
      encoder->setBuffer(buffer, 0, 0);
      float fscal = scal;
      encoder->setBytes(&fscal, sizeof(float), 1);

      MTL::Size gridSize = MTL::Size(size, 1, 1);
      NS::UInteger maxThreads = pipelineState->maxTotalThreadsPerThreadgroup();
      NS::UInteger threadsGroupDim = (size < maxThreads) ? size : maxThreads;
      MTL::Size threadgroupSize = MTL::Size(threadsGroupDim, 1, 1);
      
      encoder->dispatchThreads(gridSize, threadgroupSize);
      encoder->endEncoding();

      // 7. Execute on GPU and block until complete
      commandBuffer->commit();
      commandBuffer->waitUntilCompleted();

      return *this;
    }

    virtual BaseVector & Add (double scal, const BaseVector & v2) override
    {
      const MetalVector &mv2 = dynamic_cast<const MetalVector&>(v2);
      
      NS::Error* error = nullptr;      
      MTL::ComputePipelineState* pipelineState = device->newComputePipelineState(saxpyFunc, &error);
      
      // 6. Encode and submit compute commands
      MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
      MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();

      encoder->setComputePipelineState(pipelineState);
      encoder->setBuffer(mv2.buffer, 0, 0);
      encoder->setBuffer(buffer, 0, 1);      
      float fscal = scal;
      encoder->setBytes(&fscal, sizeof(float), 2);

      MTL::Size gridSize = MTL::Size(size, 1, 1);
      NS::UInteger maxThreads = pipelineState->maxTotalThreadsPerThreadgroup();
      NS::UInteger threadsGroupDim = (size < maxThreads) ? size : maxThreads;
      MTL::Size threadgroupSize = MTL::Size(threadsGroupDim, 1, 1);
      
      encoder->dispatchThreads(gridSize, threadgroupSize);
      encoder->endEncoding();

      // 7. Execute on GPU and block until complete
      commandBuffer->commit();
      commandBuffer->waitUntilCompleted();

      return *this;
    }
    


    

    virtual size_t Size() const  { return size; }
    virtual void * Memory () const override { return buffer->contents(); }
    virtual AutoVector CreateVector () const override
    {
      return make_unique<MetalVector>(Size());
    }

    virtual ostream& Print (ostream &ost) const override
    {
      ost << "MetalVector, size = " << size << endl;
      float * values = (float*)buffer->contents();      
      for (size_t i = 0; i < size; i++)
        ost << values[i] << "\n";
      return ost;
    }
  };
}

#endif
