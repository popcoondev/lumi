// グローバル変数
let connectionStatus = 'connecting'; // 'connecting', 'connected', 'disconnected'
let patterns = [];
let currentColorHex = '#ffff00'; // デフォルト色（黄色）

// DOMが読み込まれたら実行
document.addEventListener('DOMContentLoaded', () => {
    // モーダル用のスタイルを追加
    addModalStyles();
    
    // APIステータスの確認
    checkApiStatus();
    
    // パターンリストの取得
    fetchPatterns();
    
    // 色選択ボタンを追加
    addColorButton();
    
    // パターンボタンのイベント設定
    setupPatternButton();
});

// 接続ステータスの更新
function updateConnectionStatus(status) {
    const statusIndicator = document.querySelector('.status-indicator');
    const statusText = document.querySelector('.status-text');
    
    connectionStatus = status;
    
    if (status === 'connected') {
        statusIndicator.classList.add('connected');
        statusText.textContent = '接続済み';
    } else if (status === 'disconnected') {
        statusIndicator.classList.remove('connected');
        statusText.textContent = '切断';
    } else {
        statusIndicator.classList.remove('connected');
        statusText.textContent = '接続中...';
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
            console.log('Patterns loaded:', patterns);
        })
        .catch(error => {
            console.error('Error fetching patterns:', error);
            showStatus('パターンの取得に失敗しました', 'danger');
        });
}

// パターンボタンのイベント設定
function setupPatternButton() {
    const patternBtn = document.getElementById('patternBtn');
    patternBtn.addEventListener('click', showPatternModal);
}

// 色選択ボタンを追加
function addColorButton() {
    const colorBtn = document.createElement('button');
    colorBtn.id = 'colorBtn';
    colorBtn.className = 'btn btn-outline-success w-100';
    colorBtn.textContent = 'Color';
    
    // ボタンを追加する場所を特定
    const buttonRow = document.getElementById('resetBtn').parentElement.parentElement;
    const newCol = document.createElement('div');
    newCol.className = 'col';
    newCol.appendChild(colorBtn);
    buttonRow.appendChild(newCol);
    
    // 色選択ボタンのイベント
    colorBtn.addEventListener('click', showColorPickerModal);
}

// パターンモーダルを表示
function showPatternModal() {
    // 既存のモーダルがあれば削除
    let existingModal = document.getElementById('patternModal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // モーダルを作成
    const modal = document.createElement('div');
    modal.id = 'patternModal';
    modal.className = 'modal fade show';
    modal.style.display = 'block';
    modal.style.position = 'fixed';
    modal.style.top = '0';
    modal.style.left = '0';
    modal.style.width = '100%';
    modal.style.height = '100%';
    modal.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';
    modal.style.zIndex = '1050';
    modal.style.overflow = 'auto';
    
    // モーダルの内容を作成
    let modalContent = `
        <div class="modal-dialog modal-dialog-centered" style="position: relative; width: auto; margin: 1.75rem auto; max-width: 500px;">
            <div class="modal-content" style="position: relative; background-color: var(--bs-dark); border: 1px solid rgba(0, 0, 0, 0.2); border-radius: 0.3rem; outline: 0;">
                <div class="modal-header" style="display: flex; align-items: center; justify-content: space-between; padding: 1rem; border-bottom: 1px solid #dee2e6;">
                    <h5 class="modal-title">LEDパターン選択</h5>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close" style="background: transparent; border: 0; font-size: 1.5rem; color: white; cursor: pointer;">×</button>
                </div>
                <div class="modal-body" style="position: relative; padding: 1rem;">
                    <div class="list-group">
    `;
    
    // パターンリストを追加
    if (patterns.length > 0) {
        patterns.forEach(pattern => {
            modalContent += `
                <button type="button" class="list-group-item list-group-item-action pattern-item" data-pattern-id="${pattern.id}" style="position: relative; display: block; padding: 0.75rem 1.25rem; background-color: var(--bs-dark); color: white; border: 1px solid rgba(255, 255, 255, 0.1); cursor: pointer;">
                    ${pattern.name}
                </button>
            `;
        });
    } else {
        modalContent += `
            <div class="alert alert-info">
                パターンがありません
            </div>
        `;
    }
    
    // モーダルのフッターを追加
    modalContent += `
                    </div>
                </div>
                <div class="modal-footer" style="display: flex; align-items: center; justify-content: flex-end; padding: 1rem; border-top: 1px solid #dee2e6; gap: 0.5rem;">
                    <button type="button" class="btn btn-danger" id="stopPatternBtn">停止</button>
                    <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">閉じる</button>
                </div>
            </div>
        </div>
    `;
    
    modal.innerHTML = modalContent;
    document.body.appendChild(modal);
    
    // 背景をクリックしたらモーダルを閉じる
    modal.addEventListener('click', (e) => {
        if (e.target === modal) {
            closePatternModal();
        }
    });
    
    // 閉じるボタンのイベント
    const closeBtn = modal.querySelector('.btn-close');
    closeBtn.addEventListener('click', closePatternModal);
    
    const closeModalBtn = modal.querySelector('[data-bs-dismiss="modal"]');
    closeModalBtn.addEventListener('click', closePatternModal);
    
    // パターン項目のクリックイベント
    const patternItems = modal.querySelectorAll('.pattern-item');
    patternItems.forEach(item => {
        item.addEventListener('click', () => {
            const patternId = item.dataset.patternId;
            runPattern(patternId);
            closePatternModal();
        });
    });
    
    // パターン停止ボタンのイベント
    const stopPatternBtn = document.getElementById('stopPatternBtn');
    stopPatternBtn.addEventListener('click', () => {
        stopPattern();
        closePatternModal();
    });
}

// パターンモーダルを閉じる
function closePatternModal() {
    const modal = document.getElementById('patternModal');
    if (modal) {
        modal.remove();
    }
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
        showStatus(`パターン「${patterns.find(p => p.id == patternId)?.name || patternId}」を実行中`, 'success');
    })
    .catch(error => {
        console.error('Error running pattern:', error);
        showStatus('パターンの実行に失敗しました', 'danger');
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
        showStatus('パターンを停止しました', 'success');
    })
    .catch(error => {
        console.error('Error stopping pattern:', error);
        showStatus('パターンの停止に失敗しました', 'danger');
    });
}

// 特定のLEDセグメントを設定
function setLED(segment, state) {
    // 色を16進数から RGB に変換
    const r = parseInt(currentColorHex.substring(1, 3), 16);
    const g = parseInt(currentColorHex.substring(3, 5), 16);
    const b = parseInt(currentColorHex.substring(5, 7), 16);
    
    fetch(`/api/led/face/${segment}?r=${r}&g=${g}&b=${b}`, {
        method: 'POST'
    })
    .then(response => {
        if (!response.ok) throw new Error('通信エラーが発生しました');
        return response.json();
    })
    .then(data => {
        console.log(`LED ${segment} set to ${state ? 'ON' : 'OFF'} with color ${currentColorHex}:`, data);
    })
    .catch(error => {
        console.error('Error setting LED:', error);
        showStatus(error.message, 'danger');
    });
}

// すべてのLEDを設定
function setAllLEDs(state) {
    // 色を16進数から RGB に変換
    const r = parseInt(currentColorHex.substring(1, 3), 16);
    const g = parseInt(currentColorHex.substring(3, 5), 16);
    const b = parseInt(currentColorHex.substring(5, 7), 16);
    
    // すべての面に対して同じ色を設定
    const promises = [];
    for (let i = 0; i < 8; i++) {
        if (state) {
            promises.push(
                fetch(`/api/led/face/${i}?r=${r}&g=${g}&b=${b}`, {
                    method: 'POST'
                })
            );
        }
    }
    
    // 消灯の場合はリセットAPIを使用
    if (!state) {
        promises.push(
            fetch('/api/led/reset', {
                method: 'POST'
            })
        );
    }
    
    Promise.all(promises)
        .then(() => {
            console.log(`All LEDs set to ${state ? 'ON' : 'OFF'} with color ${currentColorHex}`);
            showStatus(`すべてのLEDを${state ? '点灯' : '消灯'}しました`, 'success');
        })
        .catch(error => {
            console.error('Error setting all LEDs:', error);
            showStatus('LEDの設定に失敗しました', 'danger');
        });
}

// ステータスメッセージを表示
function showStatus(message, type) {
    const statusEl = document.getElementById('statusMessage');
    statusEl.textContent = message;
    statusEl.className = `alert alert-${type}`;
    statusEl.classList.remove('d-none');

    setTimeout(() => {
        statusEl.classList.add('d-none');
    }, 3000);
}

// カラーピッカーモーダルを表示
function showColorPickerModal() {
    // 既存のモーダルがあれば削除
    let existingModal = document.getElementById('colorPickerModal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // モーダルを作成
    const modal = document.createElement('div');
    modal.id = 'colorPickerModal';
    modal.className = 'modal fade show';
    modal.style.display = 'block';
    modal.style.position = 'fixed';
    modal.style.top = '0';
    modal.style.left = '0';
    modal.style.width = '100%';
    modal.style.height = '100%';
    modal.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';
    modal.style.zIndex = '1050';
    modal.style.overflow = 'auto';
    
    // モーダルの内容を作成
    modal.innerHTML = `
        <div class="modal-dialog modal-dialog-centered" style="position: relative; width: auto; margin: 1.75rem auto; max-width: 500px;">
            <div class="modal-content" style="position: relative; background-color: var(--bs-dark); border: 1px solid rgba(0, 0, 0, 0.2); border-radius: 0.3rem; outline: 0;">
                <div class="modal-header" style="display: flex; align-items: center; justify-content: space-between; padding: 1rem; border-bottom: 1px solid #dee2e6;">
                    <h5 class="modal-title">色を選択</h5>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close" style="background: transparent; border: 0; font-size: 1.5rem; color: white; cursor: pointer;">×</button>
                </div>
                <div class="modal-body" style="position: relative; padding: 1rem;">
                    <div class="mb-3">
                        <label for="colorPicker" class="form-label">カラーピッカー</label>
                        <input type="color" class="form-control form-control-color w-100" id="colorPicker" value="${currentColorHex}">
                    </div>
                    <div class="color-presets d-flex flex-wrap gap-2 mb-3">
                        <button class="color-preset" style="background-color: #ff0000; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ff0000"></button>
                        <button class="color-preset" style="background-color: #00ff00; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#00ff00"></button>
                        <button class="color-preset" style="background-color: #0000ff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#0000ff"></button>
                        <button class="color-preset" style="background-color: #ffff00; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ffff00"></button>
                        <button class="color-preset" style="background-color: #00ffff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#00ffff"></button>
                        <button class="color-preset" style="background-color: #ff00ff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ff00ff"></button>
                        <button class="color-preset" style="background-color: #ffffff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ffffff"></button>
                        <button class="color-preset" style="background-color: #ff8000; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ff8000"></button>
                    </div>
                </div>
                <div class="modal-footer" style="display: flex; align-items: center; justify-content: flex-end; padding: 1rem; border-top: 1px solid #dee2e6; gap: 0.5rem;">
                    <button type="button" class="btn btn-primary" id="applyColorBtn">適用</button>
                    <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">キャンセル</button>
                </div>
            </div>
        </div>
    `;
    
    document.body.appendChild(modal);
    
    // 背景をクリックしたらモーダルを閉じる
    modal.addEventListener('click', (e) => {
        if (e.target === modal) {
            closeColorPickerModal();
        }
    });
    
    // 閉じるボタンのイベント
    const closeBtn = modal.querySelector('.btn-close');
    closeBtn.addEventListener('click', closeColorPickerModal);
    
    const cancelBtn = modal.querySelector('[data-bs-dismiss="modal"]');
    cancelBtn.addEventListener('click', closeColorPickerModal);
    
    // カラーピッカーのイベント
    const colorPicker = document.getElementById('colorPicker');
    colorPicker.addEventListener('input', (e) => {
        currentColorHex = e.target.value;
    });
    
    // プリセットカラーのイベント
    const colorPresets = modal.querySelectorAll('.color-preset');
    colorPresets.forEach(preset => {
        preset.addEventListener('click', () => {
            currentColorHex = preset.dataset.color;
            colorPicker.value = currentColorHex;
        });
    });
    
    // 適用ボタンのイベント
    const applyColorBtn = document.getElementById('applyColorBtn');
    applyColorBtn.addEventListener('click', () => {
        // LEDControllerのインスタンスを取得
        const ledController = window.ledController;
        if (ledController) {
            // 現在フォーカスされている面があれば、その面の色を変更
            if (ledController.focusedSegment !== -1) {
                ledController.setLED(ledController.focusedSegment, true);
            }
        }
        
        closeColorPickerModal();
    });
}

// カラーピッカーモーダルを閉じる
function closeColorPickerModal() {
    const modal = document.getElementById('colorPickerModal');
    if (modal) {
        modal.remove();
    }
}

// モーダル用のスタイルを追加
function addModalStyles() {
    const style = document.createElement('style');
    style.textContent = `
        .modal {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0, 0, 0, 0.5);
            z-index: 1050;
            overflow: auto;
        }
        
        .modal-dialog {
            position: relative;
            width: auto;
            margin: 1.75rem auto;
            max-width: 500px;
        }
        
        .modal-content {
            position: relative;
            background-color: var(--bs-dark);
            border: 1px solid rgba(0, 0, 0, 0.2);
            border-radius: 0.3rem;
            outline: 0;
        }
        
        .modal-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 1rem;
            border-bottom: 1px solid #dee2e6;
        }
        
        .modal-body {
            position: relative;
            padding: 1rem;
        }
        
        .modal-footer {
            display: flex;
            align-items: center;
            justify-content: flex-end;
            padding: 1rem;
            border-top: 1px solid #dee2e6;
            gap: 0.5rem;
        }
        
        .btn-close {
            background: transparent;
            border: 0;
            font-size: 1.5rem;
            color: white;
            cursor: pointer;
        }
        
        .list-group {
            display: flex;
            flex-direction: column;
            padding-left: 0;
            margin-bottom: 0;
            border-radius: 0.25rem;
        }
        
        .list-group-item {
            position: relative;
            display: block;
            padding: 0.75rem 1.25rem;
            background-color: var(--bs-dark);
            color: white;
            border: 1px solid rgba(255, 255, 255, 0.1);
            cursor: pointer;
        }
        
        .list-group-item:hover {
            background-color: var(--bs-primary);
        }
        
        .color-preset {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            border: 2px solid #fff;
            cursor: pointer;
        }
    `;
    
    document.head.appendChild(style);
}

// LEDControllerクラスの拡張
// index.htmlに既に定義されているLEDControllerクラスを拡張
document.addEventListener('DOMContentLoaded', () => {
    // LEDControllerのインスタンスが作成された後に実行
    setTimeout(() => {
        const ledController = window.ledController;
        if (ledController) {
            // LEDControllerのdrawメソッドをオーバーライド
            const originalDraw = ledController.draw;
            ledController.draw = function() {
                originalDraw.call(this);
                
                // 色付きのLEDを描画
                const outerPoints = this.getOctagonPoints(this.radius);
                const innerPoints = this.getOctagonPoints(this.innerRadius);
                
                for (let i = 0; i < 8; i++) {
                    if (this.ledStates[i]) {
                        this.ctx.beginPath();
                        this.ctx.moveTo(outerPoints[i].x, outerPoints[i].y);
                        this.ctx.lineTo(outerPoints[(i + 1) % 8].x, outerPoints[(i + 1) % 8].y);
                        this.ctx.lineTo(innerPoints[(i + 1) % 8].x, innerPoints[(i + 1) % 8].y);
                        this.ctx.lineTo(innerPoints[i].x, innerPoints[i].y);
                        this.ctx.closePath();
                        
                        // 16進数の色をRGBAに変換
                        const color = this.ledColors ? this.ledColors[i] : currentColorHex;
                        const r = parseInt(color.substring(1, 3), 16);
                        const g = parseInt(color.substring(3, 5), 16);
                        const b = parseInt(color.substring(5, 7), 16);
                        this.ctx.fillStyle = `rgba(${r}, ${g}, ${b}, 0.5)`;
                        this.ctx.fill();
                    }
                }
            };
            
            // ledColorsプロパティがなければ追加
            if (!ledController.ledColors) {
                ledController.ledColors = new Array(8).fill(currentColorHex);
            }
            
            // setLEDメソッドを拡張
            const originalSetLED = ledController.setLED;
            ledController.setLED = async function(segment, state) {
                try {
                    // APIを呼び出し
                    setLED(segment, state);
                    
                    // UIの更新
                    this.ledStates[segment] = state;
                    if (state && this.ledColors) {
                        this.ledColors[segment] = currentColorHex;
                    }
                    this.showStatus(`LED ${segment + 1}を${state ? '点灯' : '消灯'}しました`, 'success');
                } catch (error) {
                    this.showStatus(error.message, 'danger');
                }
            };
            
            // setAllLEDsメソッドを拡張
            const originalSetAllLEDs = ledController.setAllLEDs;
            ledController.setAllLEDs = async function(state) {
                try {
                    // APIを呼び出し
                    setAllLEDs(state);
                    
                    // UIの更新
                    this.ledStates.fill(state);
                    if (state && this.ledColors) {
                        for (let i = 0; i < 8; i++) {
                            this.ledColors[i] = currentColorHex;
                        }
                    }
                    this.showStatus('すべてのLEDを' + (state ? '点灯' : '消灯') + 'しました', 'success');
                } catch (error) {
                    this.showStatus(error.message, 'danger');
                }
            };
        }
    }, 500);
});
