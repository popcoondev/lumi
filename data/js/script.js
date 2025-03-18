// グローバル変数
let selectedFace = null;
let selectedColor = '#ff0000';
let patterns = [];
let connectionStatus = 'connecting'; // 'connecting', 'connected', 'disconnected'

// DOMが読み込まれたら実行
document.addEventListener('DOMContentLoaded', () => {
    // 接続ステータスの初期化
    updateConnectionStatus('connecting');
    
    // APIステータスの確認
    checkApiStatus();
    
    // パターンリストの取得
    fetchPatterns();
    
    // イベントリスナーの設定
    setupEventListeners();
});

// 接続ステータスの更新
function updateConnectionStatus(status) {
    const statusElement = document.getElementById('connection-status');
    connectionStatus = status;
    
    statusElement.className = '';
    statusElement.classList.add(status);
    
    switch (status) {
        case 'connected':
            statusElement.textContent = '接続済み';
            break;
        case 'disconnected':
            statusElement.textContent = '切断';
            break;
        default:
            statusElement.textContent = '接続中...';
    }
}

// APIステータスの確認
function checkApiStatus() {
    fetch('/api/status')
        .then(response => {
            if (response.ok) {
                updateConnectionStatus('connected');
                return response.json();
            } else {
                throw new Error('API status check failed');
            }
        })
        .then(data => {
            console.log('Device status:', data);
            
            // 定期的にステータスを確認
            setTimeout(checkApiStatus, 10000);
        })
        .catch(error => {
            console.error('Error checking API status:', error);
            updateConnectionStatus('disconnected');
            
            // 再接続を試みる
            setTimeout(checkApiStatus, 5000);
        });
}

// パターンリストの取得
function fetchPatterns() {
    fetch('/api/led/patterns')
        .then(response => {
            if (response.ok) {
                return response.json();
            } else {
                throw new Error('Failed to fetch patterns');
            }
        })
        .then(data => {
            patterns = data.patterns || [];
            updatePatternSelect();
        })
        .catch(error => {
            console.error('Error fetching patterns:', error);
        });
}

// パターン選択の更新
function updatePatternSelect() {
    const select = document.getElementById('pattern-select');
    select.innerHTML = '';
    
    if (patterns.length === 0) {
        const option = document.createElement('option');
        option.value = '';
        option.textContent = 'パターンがありません';
        select.appendChild(option);
        return;
    }
    
    patterns.forEach(pattern => {
        const option = document.createElement('option');
        option.value = pattern.id;
        option.textContent = pattern.name;
        select.appendChild(option);
    });
}

// イベントリスナーの設定
function setupEventListeners() {
    // 面ボタンのイベント
    const faceButtons = document.querySelectorAll('.face-button');
    faceButtons.forEach(button => {
        button.addEventListener('click', () => {
            // 以前の選択を解除
            document.querySelectorAll('.face-button.selected').forEach(btn => {
                btn.classList.remove('selected');
            });
            
            // 新しい選択を設定
            button.classList.add('selected');
            selectedFace = button.dataset.face;
        });
    });
    
    // カラーピッカーのイベント
    const colorPicker = document.getElementById('color-picker');
    colorPicker.addEventListener('input', () => {
        selectedColor = colorPicker.value;
    });
    
    // カラープリセットのイベント
    const colorPresets = document.querySelectorAll('.color-preset');
    colorPresets.forEach(preset => {
        preset.addEventListener('click', () => {
            selectedColor = preset.dataset.color;
            colorPicker.value = selectedColor;
        });
    });
    
    // 色適用ボタンのイベント
    const applyColorButton = document.getElementById('apply-color');
    applyColorButton.addEventListener('click', () => {
        if (selectedFace === null) {
            alert('面を選択してください');
            return;
        }
        
        applyColorToFace(selectedFace, selectedColor);
    });
    
    // LEDリセットボタンのイベント
    const resetLedsButton = document.getElementById('reset-leds');
    resetLedsButton.addEventListener('click', resetAllLeds);
    
    // パターン実行ボタンのイベント
    const runPatternButton = document.getElementById('run-pattern');
    runPatternButton.addEventListener('click', () => {
        const patternSelect = document.getElementById('pattern-select');
        const patternId = patternSelect.value;
        
        if (!patternId) {
            alert('パターンを選択してください');
            return;
        }
        
        runPattern(patternId);
    });
    
    // パターン停止ボタンのイベント
    const stopPatternButton = document.getElementById('stop-pattern');
    stopPatternButton.addEventListener('click', stopPattern);
}

// 面に色を適用
function applyColorToFace(faceId, color) {
    // #rrggbb 形式の色をRGB成分に分解
    const r = parseInt(color.substring(1, 3), 16);
    const g = parseInt(color.substring(3, 5), 16);
    const b = parseInt(color.substring(5, 7), 16);
    
    fetch(`/api/led/face/${faceId}?r=${r}&g=${g}&b=${b}`, {
        method: 'POST'
    })
    .then(response => {
        if (response.ok) {
            return response.json();
        } else {
            throw new Error('Failed to apply color');
        }
    })
    .then(data => {
        console.log('Color applied:', data);
    })
    .catch(error => {
        console.error('Error applying color:', error);
        alert('色の適用に失敗しました');
    });
}

// 全てのLEDをリセット
function resetAllLeds() {
    fetch('/api/led/reset', {
        method: 'POST'
    })
    .then(response => {
        if (response.ok) {
            return response.json();
        } else {
            throw new Error('Failed to reset LEDs');
        }
    })
    .then(data => {
        console.log('LEDs reset:', data);
    })
    .catch(error => {
        console.error('Error resetting LEDs:', error);
        alert('LEDのリセットに失敗しました');
    });
}

// パターンを実行
function runPattern(patternId) {
    fetch(`/api/led/pattern/${patternId}`, {
        method: 'POST'
    })
    .then(response => {
        if (response.ok) {
            return response.json();
        } else {
            throw new Error('Failed to run pattern');
        }
    })
    .then(data => {
        console.log('Pattern running:', data);
    })
    .catch(error => {
        console.error('Error running pattern:', error);
        alert('パターンの実行に失敗しました');
    });
}

// パターンを停止
function stopPattern() {
    fetch('/api/led/stop', {
        method: 'POST'
    })
    .then(response => {
        if (response.ok) {
            return response.json();
        } else {
            throw new Error('Failed to stop pattern');
        }
    })
    .then(data => {
        console.log('Pattern stopped:', data);
    })
    .catch(error => {
        console.error('Error stopping pattern:', error);
        alert('パターンの停止に失敗しました');
    });
}
