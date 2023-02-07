# porte
**p**anda's **o**ffline **r**ay **t**racing **e**ngine

* 渲染算法，效果
  - [ ] 路径追踪(PT,path tracing)
  - [ ] 体路径追踪(VPT,volume path tracing)
  - [ ] 双向路径追踪(BDPT,bidirectional path tracing)
  - [ ] 梅特波利斯光照传输(MLT,metropolis light transport)
  - [ ] 随机渐进光子映射(SPPM,stochastic progress photon mapping)
  - [ ] 光子映射与双向路径追踪结合(VCM,vertex connection and merging)
  - [ ] practical path guiding
  - [ ] 光的色散

* 场景模型解析
  - [ ] xml文件解析
  - [ ] obj模型加载
  - [ ] ply模型加载

* 几何相关
  - [ ] watertight三角形与ray求交算法
  - [ ] 经典三角形与ray求交算法
  - [ ] 球体，圆柱，圆锥，圆盘
  - [ ] 三角形网格，quad，cube
  - [ ] 表面细分
  - [ ] 曲线(curve)
  - [ ] 抛物面(Paraboloids)，双曲面(Hyperboloids)

* BSDF，材质相关
  - [ ] 次表面散射BSSRDF
  - [ ] 傅里叶BSDF
  - [ ] 迪士尼材质(Disney material)
  - [ ] microfacet BRDF
  - [ ] lambertian 反射透射
  - [ ] Specular 反射透射
  - [ ] fresnel 高光
  - [ ] Oren-Nayer
  - [ ] Fresnel Blend(用于模拟漫反射物体表面有一层光泽反射层)
  - [ ] bump map
  - [ ] normal map

* 相机
  - [ ] 透视相机
  - [ ] 正交相机
  - [ ] 真实相机
  - [ ] 环境相机
* 数学
  - [ ] 四元数，矩阵，transform，animated transform
  - [ ] 各种几何表面采样(待完善)
  - [ ] 离散一维分布，二维分布

* 采样器
  - [ ] 随机采样器(random sampler)
  - [ ] halton采样器(halton sampler)
  - [ ] 分层采样器(stratified sampler)
  - [ ] sobol采样器(sobol sampler)
  - [ ] MaxMinDistSampler
  - [ ] ZeroTwoSequenceSampler

* 性能优化
  - [ ] 内存池
  - [ ] 针对cache line优化(内存重排)
  - [ ] 向量化
  - [ ] 接入embree
  - [ ] 误差管理优化(详见rtg第六章)
  - [ ] 实例化
  - [ ] 优化几何结构，减少内存占用
 
* 滤波器(filter)
  - [ ] 高斯滤波器(gaussian filter)
  - [ ] 盒式滤波器(box filter)
  - [ ] sinc filter
  - [ ] mitchell filter
  - [ ] 三角滤波器(triangle filter)
* 参与介质
  - [ ] 均匀介质(homogeneous)
  - [ ] 非均匀介质(heterogeneous)

* 光源
  - [ ] 点光源(point light)
  - [ ] 面光源(diffuse area light)
  - [ ] 聚光灯(spot light)
  - [ ] 环境光(environment)
  - [ ] 平行光(distant light))
  - [ ] 投影仪(projection light)
  - [ ] 纹理面光源(texture area light)

* 其他
  - [ ] 统计机制
