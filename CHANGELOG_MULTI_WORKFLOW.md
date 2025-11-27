# 多工作流支持更新日志

## 更新概述

从单工作流配置升级到支持多工作流管理的可展开式界面。

## 主要变更

### 1. 数据结构重构

**旧版本：**
```javascript
let workflowConfig = {
    platform: {...},
    algo: {...},
    app: {...}
};
```

**新版本：**
```javascript
let workflows = [
    {
        id: 'wf_1234567890_1',
        name: '默认检测工作流',
        active: true,
        config: {
            platform: {...},
            algo: {...},
            app: {...}
        }
    }
];
let activeWorkflowId = null;
let currentWorkflowId = null;
```

### 2. UI 组件更新

#### 2.1 工作流配置页面

- **新增功能：**
  - ➕ 新建工作流按钮
  - 📤 导出全部工作流按钮
  - 可展开/收缩的工作流列表（Accordion 风格）
  - 每个工作流独立的操作按钮（激活、编辑、删除）

- **展示内容：**
  - 工作流名称和状态（✅ 已激活 / ⏸️ 未激活）
  - 工作流概要信息（4卡片布局）
  - 内联推理流程图
  - 完整配置参数列表

#### 2.2 配置编辑模态框

- **新增字段：**
  - 工作流名称输入框

- **更新逻辑：**
  - 支持编辑指定 ID 的工作流
  - 保存时更新指定工作流并刷新列表

### 3. 核心函数

#### 3.1 工作流管理函数

```javascript
// 数据操作
loadWorkflows()               // 从 localStorage 加载工作流列表
saveWorkflows()               // 保存工作流列表到 localStorage
initDefaultWorkflows()        // 初始化默认工作流

// 工具函数
getWorkflowById(id)          // 根据 ID 获取工作流
getActiveWorkflow()           // 获取当前激活的工作流

// UI 渲染
renderWorkflowList()          // 渲染工作流列表（Accordion）
renderWorkflowSummary()       // 渲染激活工作流的摘要
buildWorkflowStepsHTML(config) // 生成内联流程图 HTML

// 用户操作
toggleWorkflow(id)            // 展开/收缩工作流
addNewWorkflow()              // 新建工作流
activateWorkflow(id)          // 激活指定工作流
deleteWorkflow(id)            // 删除工作流
openConfigModal(id)           // 打开编辑模态框
```

#### 3.2 配置导入/导出

```javascript
exportAllWorkflows()          // 导出所有工作流为 JSON
exportConfig()                // 导出当前激活的工作流（兼容旧版）
importConfig(event)           // 导入工作流配置
                              // - 支持导入单个工作流配置
                              // - 支持导入多工作流数组
```

### 4. 样式更新

新增 CSS 类：

```css
.workflow-list              // 工作流列表容器
.workflow-item              // 单个工作流项
.workflow-header            // 工作流头部（可点击展开）
.workflow-header.active     // 已展开状态
.workflow-expand-icon       // 展开图标
.workflow-expand-icon.expanded  // 已展开图标旋转
.workflow-body              // 工作流内容区（可折叠）
.workflow-body.expanded     // 已展开状态
.workflow-actions           // 操作按钮组
.workflow-action-btn        // 操作按钮样式
```

### 5. 数据持久化

- **存储键名：** `nndeploy_workflows`（替代旧的 `workflowConfig`）
- **存储格式：** JSON 数组
- **自动保存时机：**
  - 新建工作流
  - 激活工作流
  - 删除工作流
  - 编辑工作流配置
  - 导入工作流

### 6. 向后兼容

- 保留 `exportConfig()` 函数用于导出单个工作流
- 导入时自动识别旧格式配置并转换为新格式
- 首次加载时如果没有工作流数据，自动创建默认工作流

## 用户操作流程

### 新建工作流
1. 点击"➕ 新建工作流"按钮
2. 系统创建基于默认模板的新工作流
3. 新工作流自动展开并可编辑

### 编辑工作流
1. 点击工作流的"✏️ 编辑"按钮
2. 在模态框中修改配置
3. 保存后自动刷新列表

### 激活工作流
1. 点击工作流的"✅ 激活"按钮
2. 之前激活的工作流自动变为未激活状态
3. 摘要区显示新激活的工作流信息

### 删除工作流
1. 点击工作流的"🗑️ 删除"按钮
2. 确认后删除
3. 如果删除的是激活工作流，自动激活第一个工作流

### 展开/收缩
1. 点击工作流头部区域
2. 自动展开当前工作流，收缩其他工作流
3. 平滑过渡动画

### 导出/导入
- **导出全部：** 点击"📤 导出全部"，下载所有工作流配置
- **导入配置：** 点击"📥 导入配置"，选择 JSON 文件
  - 支持导入单个工作流配置
  - 支持导入多工作流配置

## 技术细节

### Accordion 实现原理

```javascript
function toggleWorkflow(id) {
    const workflow = getWorkflowById(id);
    if (!workflow) return;
    
    // 如果当前已展开，则收缩
    if (workflow.expanded) {
        workflow.expanded = false;
    } else {
        // 收缩所有其他工作流
        workflows.forEach(wf => wf.expanded = false);
        // 展开当前工作流
        workflow.expanded = true;
    }
    
    renderWorkflowList();
}
```

### 激活工作流逻辑

```javascript
function activateWorkflow(id) {
    // 取消所有激活状态
    workflows.forEach(wf => wf.active = false);
    
    // 激活指定工作流
    const workflow = getWorkflowById(id);
    if (workflow) {
        workflow.active = true;
        activeWorkflowId = id;
    }
    
    saveWorkflows();
    renderWorkflowList();
    renderWorkflowSummary();
}
```

### 数据验证

导入时验证数据结构：

```javascript
// 多工作流格式验证
if (Array.isArray(imported)) {
    for (let wf of imported) {
        if (!wf.id || !wf.name || !wf.config) {
            throw new Error('工作流格式不正确');
        }
        if (!wf.config.platform || !wf.config.algo || !wf.config.app) {
            throw new Error('工作流配置格式不正确');
        }
    }
}

// 单工作流格式验证
else if (imported.platform && imported.algo && imported.app) {
    // 转换为新格式
}
```

## 文件变更清单

- ✅ `index.html` - 完整的多工作流支持实现
  - CSS 样式更新（Accordion 布局）
  - HTML 结构更新（工作流列表容器）
  - JavaScript 逻辑重构（数据模型、渲染、管理）

## 测试建议

### 功能测试
- [ ] 新建工作流
- [ ] 编辑工作流配置
- [ ] 激活/取消激活工作流
- [ ] 删除工作流
- [ ] 展开/收缩工作流
- [ ] 导出全部工作流
- [ ] 导入多工作流配置
- [ ] 导入单工作流配置（旧格式）
- [ ] 页面刷新后数据持久化

### 边界情况测试
- [ ] 删除最后一个工作流
- [ ] 删除当前激活的工作流
- [ ] 导入空配置
- [ ] 导入格式错误的配置
- [ ] 同时展开多个工作流（应自动收缩其他）

## 升级指南

### 从旧版本升级

1. **数据迁移（自动）：**
   - 首次加载时检测旧的 `workflowConfig`
   - 如果存在，自动创建默认工作流列表

2. **用户操作变化：**
   - 工作流配置页面改为列表视图
   - 需要先激活工作流才能在摘要区查看
   - 编辑时需要点击具体工作流的编辑按钮

3. **导出格式变化：**
   - 全量导出格式从单对象变为数组
   - 单工作流导出保持兼容

## 已知限制

1. 工作流名称暂不支持重复性检查
2. 工作流顺序暂不支持拖拽排序
3. 工作流暂不支持复制/克隆功能

## 后续优化建议

1. **工作流管理增强：**
   - 支持工作流复制/克隆
   - 支持工作流重命名
   - 支持工作流拖拽排序
   - 工作流名称唯一性验证

2. **UI 优化：**
   - 添加搜索/筛选功能
   - 支持工作流分组
   - 支持工作流标签

3. **导入导出优化：**
   - 支持导出单个工作流
   - 支持批量导入时合并/替换选项
   - 导出文件名包含工作流名称

## 更新时间

2024年（实际日期请根据系统时间填写）

## 维护者

NNDeploy Frontend Team
