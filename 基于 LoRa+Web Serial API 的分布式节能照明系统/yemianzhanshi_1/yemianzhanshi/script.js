// 初始化灯光状态
const lights = [
    { id: 'light1', active: false, intensity: 0 },
    { id: 'light2', active: false, intensity: 0 },
    { id: 'light3', active: false, intensity: 0 },
    { id: 'light4', active: false, intensity: 0 },
    { id: 'light5', active: false, intensity: 0 },
    { id: 'light6', active: false, intensity: 0 },
    { id: 'light7', active: false, intensity: 0 },
    { id: 'light8', active: false, intensity: 0 }
];

// 初始化传感器数据
let naturalLight = 500;
let peopleCount = 0;

// 控制模式：auto 或 manual
let controlMode = 'auto';

// 初始化历史数据
const historyData = {
    energy: [],
    naturalLight: [],
    peopleCount: [],
    activeLights: [],
    timestamps: []
};

// 初始化能耗数据
const energyData = {
    labels: Array.from({ length: 20 }, (_, i) => i),
    datasets: [{
        label: '能耗 (W)',
        data: Array(20).fill(0),
        borderColor: '#3498db',
        backgroundColor: 'rgba(52, 152, 219, 0.1)',
        tension: 0.4,
        fill: true
    }]
};

// 初始化自然光数据
const naturalLightData = {
    labels: Array.from({ length: 20 }, (_, i) => i),
    datasets: [{
        label: '自然光 (lux)',
        data: Array(20).fill(0),
        borderColor: '#f39c12',
        backgroundColor: 'rgba(243, 156, 18, 0.1)',
        tension: 0.4,
        fill: true
    }]
};

// 初始化人流量数据
const peopleCountData = {
    labels: Array.from({ length: 20 }, (_, i) => i),
    datasets: [{
        label: '人流量',
        data: Array(20).fill(0),
        borderColor: '#27ae60',
        backgroundColor: 'rgba(39, 174, 96, 0.1)',
        tension: 0.4,
        fill: true
    }]
};

// ==================== 串口通信相关 ====================
let port = null;
let reader = null;
let writer = null;
let isConnected = false;

// 初始化图表
const ctx = document.getElementById('energy-chart').getContext('2d');
const energyChart = new Chart(ctx, {
    type: 'line',
    data: energyData,
    options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: {
            y: { beginAtZero: true, title: { display: true, text: '能耗 (W)' } },
            x: { title: { display: true, text: '时间 (分钟)' } }
        }
    }
});

const naturalLightCtx = document.getElementById('natural-light-chart').getContext('2d');
const naturalLightChart = new Chart(naturalLightCtx, {
    type: 'line',
    data: naturalLightData,
    options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: { y: { beginAtZero: true, title: { display: true, text: '自然光 (lux)' } }, x: { display: false } },
        plugins: { legend: { display: false } }
    }
});

const peopleCountCtx = document.getElementById('people-count-chart').getContext('2d');
const peopleCountChart = new Chart(peopleCountCtx, {
    type: 'line',
    data: peopleCountData,
    options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: { y: { beginAtZero: true, title: { display: true, text: '人流量' } }, x: { display: false } },
        plugins: { legend: { display: false } }
    }
});

// ==================== 串口函数 ====================
function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function connectSerial() {
    try {
        if (!('serial' in navigator)) {
            alert('您的浏览器不支持串口通信，请使用Chrome浏览器');
            return;
        }
        port = await navigator.serial.requestPort();
        await port.open({ baudRate: 9600, dataBits: 8, stopBits: 1, parity: 'none' });
        reader = port.readable.getReader();
        writer = port.writable.getWriter();
        isConnected = true;
        console.log('串口已连接');
        const serialBtn = document.getElementById('serial-btn');
        if (serialBtn) serialBtn.textContent = '断开串口';
        receiveSerialData();
    } catch (error) {
        console.error('连接串口失败:', error);
        alert('连接串口失败: ' + error.message);
    }
}

async function disconnectSerial() {
    try {
        if (reader) { await reader.cancel(); reader.releaseLock(); }
        if (writer) { await writer.close(); }
        if (port) { await port.close(); }
        port = null; reader = null; writer = null;
        isConnected = false;
        console.log('串口已断开');
        const serialBtn = document.getElementById('serial-btn');
        if (serialBtn) serialBtn.textContent = '连接串口';
    } catch (error) {
        console.error('断开串口失败:', error);
    }
}

async function receiveSerialData() {
    const decoder = new TextDecoder();
    let buffer = '';
    try {
        while (isConnected && port && port.readable) {
            const { value, done } = await reader.read();
            if (done) break;
            buffer += decoder.decode(value);
            // 同时识别 ] 和 ) 作为帧结束符
            let frames = [];
            let lastIndex = 0;
            for (let i = 0; i < buffer.length; i++) {
                if (buffer[i] === ']' || buffer[i] === ')') {
                    frames.push(buffer.substring(lastIndex, i + 1));
                    lastIndex = i + 1;
                }
            }
            buffer = buffer.substring(lastIndex);
            for (const frame of frames) {
                if (frame.trim()) {
                    console.log('📥 收到:', frame);
                    parseSTM32Data(frame);
                }
            }
        }
    } catch (error) {
        console.error('接收数据失败:', error);
    }
}

function parseSTM32Data(data) {
    // 确定括号类型
    const isSquare = data[0] === '[' && data[data.length - 1] === ']';
    const isRound = data[0] === '(' && data[data.length - 1] === ')';
    if (!isSquare && !isRound) return;

    const content = data.slice(1, -1);
    // 解析档位指令 [gearX,No:XX] 或 (gearX,No:XX)
    const gearMatch = content.match(/gear(\d+),No:(\d+)/);
    if (gearMatch && controlMode === 'auto') {
        const gear = parseInt(gearMatch[1]);
        if (isSquare) {
            // 方括号：第一组
            updateGroupLights('first', gear);
            console.log(`自动模式：第一组档位 ${gear}`);
        } else if (isRound) {
            // 圆括号：第二组
            updateGroupLights('second', gear);
            console.log(`自动模式：第二组档位 ${gear}`);
        }
        return;
    }

    // 原有的 node 和 mode 解析（保持不变）
    const parts = content.split(',');
    if (parts[0] === 'node' && parts.length >= 3) {
        const lightValue = parseInt(parts[2]);
        naturalLight = lightValue;
        document.getElementById('natural-light').textContent = lightValue;
        document.getElementById('manual-natural-light').value = lightValue;
        updateEnergy();
        updateOverview();
    }
    if (parts[0] === 'mode' && parts.length >= 3) {
        const modeValue = parts[1];
        const newMode = modeValue === '1' ? 'auto' : 'manual';
        if (newMode !== controlMode) {
            setControlMode(newMode);
        }
    }
}

// 方括号指令：自动重发5次
async function sendToSTM32(data) {
    if (!isConnected || !writer) {
        console.warn('串口未连接');
        return false;
    }
    try {
        const encoder = new TextEncoder();
        const frame = '[' + data + ']';
        // 连续发送 5 次
        for (let i = 1; i <= 5; i++) {
            await writer.write(encoder.encode(frame));
            console.log(`📤 发送(${i}/5):`, frame);
            await delay(50); // 每次间隔 50ms
        }
        return true;
    } catch (error) {
        console.error('发送失败:', error);
        return false;
    }
}

// 原始指令（圆括号等）：自动重发5次
async function sendRawCommand(rawString) {
    if (!isConnected || !writer) {
        console.warn('串口未连接');
        return false;
    }
    try {
        const encoder = new TextEncoder();
        // 连续发送 5 次
        for (let i = 1; i <= 5; i++) {
            await writer.write(encoder.encode(rawString));
            console.log(`📤 发送(${i}/5):`, rawString);
            await delay(50);
        }
        return true;
    } catch (error) {
        console.error('发送失败:', error);
        return false;
    }
}
// ==================== 核心控制函数 ====================

function updateLightUI(light) {
    const lightElement = document.getElementById(light.id);
    if (!lightElement) return;
    const intensitySlider = document.getElementById('intensity' + light.id.replace('light', ''));
    const intensityValue = document.getElementById('intensity-value' + light.id.replace('light', ''));
    if (light.active) {
        lightElement.classList.add('active');
        lightElement.querySelector('.light-status').textContent = '开启';
        const power = (light.intensity / 100) * 10;
        lightElement.querySelector('.light-power').textContent = power.toFixed(1) + 'W';
        if (intensitySlider) intensitySlider.value = light.intensity;
        if (intensityValue) intensityValue.textContent = light.intensity + '%';
    } else {
        lightElement.classList.remove('active');
        lightElement.querySelector('.light-status').textContent = '关闭';
        lightElement.querySelector('.light-power').textContent = '0W';
        if (intensitySlider) intensitySlider.value = 0;
        if (intensityValue) intensityValue.textContent = '0%';
    }
}

function setOverallGear(gear) {
    for (let i = 0; i < lights.length; i++) {
        const groupIndex = i < 4 ? i : i - 4;
        if (groupIndex < gear) {
            lights[i].active = true;
            lights[i].intensity = 100;
        } else {
            lights[i].active = false;
            lights[i].intensity = 0;
        }
    }
    lights.forEach(light => updateLightUI(light));

    if (isConnected) {
        sendToSTM32(`gear,0,${gear}`);
        sendRawCommand(`(gear,0,${gear})`);
    }
    updateEnergy();
    updateOverview();
    clearGroupHighlights('group1');
    clearGroupHighlights('group2');
}
async function setGroupGear(groupId, gear) {
    const startIdx = groupId === 1 ? 0 : 4;
    const endIdx = startIdx + 4;
    for (let i = startIdx; i < endIdx; i++) {
        const idxInGroup = i - startIdx;
        if (idxInGroup < gear) {
            lights[i].active = true;
            lights[i].intensity = 100;
        } else {
            lights[i].active = false;
            lights[i].intensity = 0;
        }
        updateLightUI(lights[i]);
    }
    updateEnergy();
    updateOverview();
    clearGroupHighlights('global');
    if (groupId === 1) clearGroupHighlights('group2');
    else clearGroupHighlights('group1');

    // 发送串口指令序列
    if (isConnected) {
        if (groupId === 1) {
            // 第一组：发送 [gear,1,x], [gear,2,x], [gear,3,x]，每个指令本身会重发5次（在sendToSTM32内部）
            for (let i = 1; i <= 3; i++) {
                await sendToSTM32(`gear,${i},${gear}`);
                await delay(30); // 三次之间延时100ms
            }
        } else {
            // 第二组：发送 (gear,4,x), (gear,5,x), (gear,6,x)
            for (let i = 4; i <= 6; i++) {
                await sendRawCommand(`(gear,${i},${gear})`);
                await delay(30);
            }
        }
    }
}
// 更新指定组的灯光（不改变另一组）
function updateGroupLights(group, gear) {
    // group: 'first' 或 'second'
    // gear: 1~4，表示该组亮前 gear 个灯，强度100%
    const startIdx = group === 'first' ? 0 : 4;
    const endIdx = startIdx + 4;
    for (let i = startIdx; i < endIdx; i++) {
        const idxInGroup = i - startIdx;
        if (idxInGroup < gear) {
            lights[i].active = true;
            lights[i].intensity = 100;
        } else {
            lights[i].active = false;
            lights[i].intensity = 0;
        }
        updateLightUI(lights[i]);
    }
    updateEnergy();
    updateOverview();
}

function clearGroupHighlights(group) {
    let selector = '';
    if (group === 'global') selector = '.global-gear';
    else if (group === 'group1') selector = '.group1-gear';
    else if (group === 'group2') selector = '.group2-gear';
    else return;
    document.querySelectorAll(selector).forEach(btn => {
        btn.classList.remove('active-gear');
        btn.style.backgroundColor = '';
    });
}

function highlightButton(btn, groupSelector) {
    document.querySelectorAll(groupSelector).forEach(b => {
        b.classList.remove('active-gear');
        b.style.backgroundColor = '';
    });
    btn.classList.add('active-gear');
    btn.style.backgroundColor = '#2ecc71';
}

// ==================== 原有业务函数 ====================
function updateTime() {
    const now = new Date();
    const timeString = now.toLocaleString('zh-CN', {
        year: 'numeric', month: '2-digit', day: '2-digit',
        hour: '2-digit', minute: '2-digit', second: '2-digit'
    });
    document.getElementById('current-time').textContent = timeString;
}

function updateLights() {
    if (controlMode === 'auto') {
        // 如果已连接串口，前端不再自己计算灯光，完全听从单片机指令
        if (isConnected) {
            return; // 等待单片机发送档位指令来更新UI
        }
        // 未连接串口时，模拟自动调节（原有代码保持不变）
        let lightsToTurnOn = 0;
        let lightIntensity = 100;
        if (naturalLight < 100) { lightsToTurnOn = 8; lightIntensity = 100; }
        else if (naturalLight < 300) { lightsToTurnOn = 6; lightIntensity = 80; }
        else if (naturalLight < 500) { lightsToTurnOn = 4; lightIntensity = 60; }
        else if (naturalLight < 700) { lightsToTurnOn = 2; lightIntensity = 40; }
        else { lightsToTurnOn = 1; lightIntensity = 20; }
        const lightsPerGroup = Math.floor(lightsToTurnOn / 2);
        const extraLight = lightsToTurnOn % 2;
        for (let i = 0; i < lights.length; i++) {
            const groupIdx = i < 4 ? i : i - 4;
            let shouldBeOn = (i < 4) ? (groupIdx < lightsPerGroup + extraLight) : (groupIdx < lightsPerGroup);
            lights[i].active = shouldBeOn;
            if (shouldBeOn) lights[i].intensity = lightIntensity;
            else lights[i].intensity = 0;
            updateLightUI(lights[i]);
        }
        updateEnergy();
        updateOverview();
    }
}

function updateLightButtonStatus() {
    const allManualBtns = document.querySelectorAll('.global-gear, .group1-gear, .group2-gear');
    const modeTip = document.getElementById('mode-tip');
    if (controlMode === 'auto') {
        allManualBtns.forEach(btn => btn.disabled = true);
        if (modeTip) modeTip.style.display = 'inline';
    } else {
        allManualBtns.forEach(btn => btn.disabled = false);
        if (modeTip) modeTip.style.display = 'none';
    }
}

function updateSensors() {
    if (!isConnected) {
        naturalLight = Math.max(0, Math.min(1000, naturalLight + (Math.random() - 0.5) * 30));
        peopleCount = Math.max(0, Math.min(20, peopleCount + (Math.random() - 0.5) * 5));
    } else {
        peopleCount = Math.max(0, Math.min(20, peopleCount + (Math.random() - 0.5) * 5));
    }
    document.getElementById('natural-light').textContent = Math.round(naturalLight);
    document.getElementById('manual-natural-light').value = naturalLight;
    document.getElementById('people-count').textContent = Math.round(peopleCount);
}

function updateEnergy() {
    let currentEnergy = 0, activeLights = 0;
    lights.forEach(light => {
        if (light.active) {
            activeLights++;
            currentEnergy += (light.intensity / 100) * 10;
        }
    });
    const now = new Date();
    historyData.energy.push(currentEnergy);
    historyData.naturalLight.push(naturalLight);
    historyData.peopleCount.push(peopleCount);
    historyData.activeLights.push(activeLights);
    historyData.timestamps.push(now.toLocaleString('zh-CN'));
    if (historyData.energy.length > 60) {
        historyData.energy.shift(); historyData.naturalLight.shift();
        historyData.peopleCount.shift(); historyData.activeLights.shift();
        historyData.timestamps.shift();
    }
    const range = energyData.labels.length;
    energyData.datasets[0].data = historyData.energy.slice(-range);
    energyChart.update();
    naturalLightData.datasets[0].data = historyData.naturalLight.slice(-range);
    naturalLightChart.update();
    peopleCountData.datasets[0].data = historyData.peopleCount.slice(-range);
    peopleCountChart.update();
    updateTable();
}

function updateTable() {
    const tableBody = document.getElementById('energy-table-body');
    tableBody.innerHTML = '';
    const timeRange = parseInt(document.getElementById('time-range').value);
    const recentData = Math.min(timeRange, historyData.energy.length);
    for (let i = historyData.energy.length - recentData; i < historyData.energy.length; i++) {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${historyData.timestamps[i]}</td>
            <td>${historyData.energy[i].toFixed(2)}</td>
            <td>${historyData.activeLights[i]}</td>
            <td>${Math.round(historyData.naturalLight[i])}</td>
            <td>${Math.round(historyData.peopleCount[i])}</td>
        `;
        tableBody.appendChild(row);
    }
}

function updateOverview() {
    const totalEnergy = historyData.energy.reduce((sum, val) => sum + val, 0) / 60000;
    document.getElementById('total-energy').textContent = totalEnergy.toFixed(3);
    const activeLights = lights.filter(light => light.active).length;
    document.getElementById('active-lights').textContent = activeLights;
    if (historyData.naturalLight.length > 0) {
        const avgNaturalLight = historyData.naturalLight.reduce((a,b)=>a+b,0) / historyData.naturalLight.length;
        document.getElementById('avg-natural-light').textContent = Math.round(avgNaturalLight);
    }
    if (historyData.peopleCount.length > 0) {
        const avgPeopleCount = historyData.peopleCount.reduce((a,b)=>a+b,0) / historyData.peopleCount.length;
        document.getElementById('avg-people-count').textContent = avgPeopleCount.toFixed(1);
    }
}

function downloadExcel() {
    const data = [['时间', '能耗 (W)', '活跃灯光', '自然光 (lux)', '人流量']];
    for (let i = 0; i < historyData.energy.length; i++) {
        data.push([
            historyData.timestamps[i],
            historyData.energy[i].toFixed(2),
            historyData.activeLights[i],
            Math.round(historyData.naturalLight[i]),
            Math.round(historyData.peopleCount[i])
        ]);
    }
    const wb = XLSX.utils.book_new();
    const ws = XLSX.utils.aoa_to_sheet(data);
    XLSX.utils.book_append_sheet(wb, ws, '能耗数据');
    XLSX.writeFile(wb, `能耗数据_${new Date().toISOString().slice(0,10)}.xlsx`);
}

function changeTimeRange() {
    const range = parseInt(document.getElementById('time-range').value);
    energyData.labels = Array.from({ length: range }, (_, i) => i);
    let recentEnergyData = historyData.energy.slice(-range);
    while (recentEnergyData.length < range) recentEnergyData.unshift(0);
    energyData.datasets[0].data = recentEnergyData;
    energyChart.update();
    naturalLightData.labels = Array.from({ length: range }, (_, i) => i);
    let recentNaturalLightData = historyData.naturalLight.slice(-range);
    while (recentNaturalLightData.length < range) recentNaturalLightData.unshift(0);
    naturalLightData.datasets[0].data = recentNaturalLightData;
    naturalLightChart.update();
    peopleCountData.labels = Array.from({ length: range }, (_, i) => i);
    let recentPeopleCountData = historyData.peopleCount.slice(-range);
    while (recentPeopleCountData.length < range) recentPeopleCountData.unshift(0);
    peopleCountData.datasets[0].data = recentPeopleCountData;
    peopleCountChart.update();
    updateTable();
}



function adjustIntensity(lightId, value) {
    if (controlMode === 'auto') setControlMode('manual');
    const light = lights.find(l => l.id === lightId);
    if (light) {
        light.intensity = parseInt(value);
        updateLightUI(light);
        updateEnergy();
        updateOverview();
    }
}

function setControlMode(mode) {
    controlMode = mode;
    document.getElementById('auto-mode').classList.remove('active');
    document.getElementById('manual-mode').classList.remove('active');
    document.getElementById(mode + '-mode').classList.add('active');
    updateLightButtonStatus();
    if (mode === 'auto') {
        updateLights();
        updateEnergy();
        updateOverview();
    }
}

function updateAll() {
    updateTime();
    updateLights();
    updateSensors();
    updateEnergy();
    updateOverview();
}

// ==================== 事件绑定 ====================
document.getElementById('download-btn').addEventListener('click', downloadExcel);
document.getElementById('time-range').addEventListener('change', changeTimeRange);
document.getElementById('refresh-btn').addEventListener('click', () => {
    energyData.datasets[0].data = Array(energyData.labels.length).fill(0);
    energyChart.update();
    naturalLightData.datasets[0].data = Array(naturalLightData.labels.length).fill(0);
    naturalLightChart.update();
    peopleCountData.datasets[0].data = Array(peopleCountData.labels.length).fill(0);
    peopleCountChart.update();
    historyData.energy = []; historyData.naturalLight = []; historyData.peopleCount = [];
    historyData.activeLights = []; historyData.timestamps = [];
    updateTable(); updateOverview();
});
document.getElementById('cover-sensor').addEventListener('click', () => {
    naturalLight = 100;
    document.getElementById('manual-natural-light').value = naturalLight;
    updateSensors(); updateLights(); updateEnergy(); updateOverview();
});
document.getElementById('uncover-sensor').addEventListener('click', () => {
    naturalLight = 800;
    document.getElementById('manual-natural-light').value = naturalLight;
    updateSensors(); updateLights(); updateEnergy(); updateOverview();
});
document.getElementById('auto-mode').addEventListener('click', () => {
    sendToSTM32('mode,1');
});
document.getElementById('manual-mode').addEventListener('click', () => {
    sendToSTM32('mode,0');
});

document.querySelectorAll('.global-gear').forEach(btn => {
    btn.addEventListener('click', (e) => {
        const gear = parseInt(btn.getAttribute('data-gear'));
        setOverallGear(gear);
        highlightButton(btn, '.global-gear');
    });
});
document.querySelectorAll('.group1-gear').forEach(btn => {
    btn.addEventListener('click', (e) => {
        const gear = parseInt(btn.getAttribute('data-gear'));
        setGroupGear(1, gear);
        highlightButton(btn, '.group1-gear');
    });
});
document.querySelectorAll('.group2-gear').forEach(btn => {
    btn.addEventListener('click', (e) => {
        const gear = parseInt(btn.getAttribute('data-gear'));
        setGroupGear(2, gear);
        highlightButton(btn, '.group2-gear');
    });
});

window.updateNaturalLight = function() {
    const manualValue = parseInt(document.getElementById('manual-natural-light').value);
    if (!isNaN(manualValue) && manualValue >= 0 && manualValue <= 1000) {
        naturalLight = manualValue;
        updateSensors();
        updateLights();
        updateEnergy();
        updateOverview();
    }
};

function addSerialButton() {
    const headerInfo = document.querySelector('.header-info');
    if (!headerInfo) return;
    if (document.getElementById('serial-btn')) return;
    const serialBtn = document.createElement('button');
    serialBtn.id = 'serial-btn';
    serialBtn.className = 'download-btn';
    serialBtn.textContent = '连接串口';
    serialBtn.style.backgroundColor = '#3498db';
    serialBtn.style.marginLeft = '10px';
    serialBtn.addEventListener('click', () => {
        if (isConnected) disconnectSerial();
        else connectSerial();
    });
    headerInfo.appendChild(serialBtn);
}

document.addEventListener('DOMContentLoaded', () => {
    addSerialButton();
    clearGroupHighlights('global');
    clearGroupHighlights('group1');
    clearGroupHighlights('group2');
});

setInterval(updateAll, 1000);
updateAll();
updateLightButtonStatus();