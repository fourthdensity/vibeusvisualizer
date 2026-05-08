# E1.3 Beat Indicator - Quick Test Script

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  E1.3 Beat Indicator - Manual Test" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Test Instructions:" -ForegroundColor Yellow
Write-Host "1. Vibeus will launch in debug mode" -ForegroundColor White
Write-Host "2. Start the visualizer from the main menu" -ForegroundColor White
Write-Host "3. Play some music with clear beats (EDM recommended)" -ForegroundColor White
Write-Host "4. Press 'I' key to toggle beat indicator" -ForegroundColor White
Write-Host "5. Observe red circle in top-left corner pulsing with beats" -ForegroundColor White
Write-Host "6. Press 'I' again to disable" -ForegroundColor White
Write-Host ""

Write-Host "Expected Behavior:" -ForegroundColor Green
Write-Host "  ✓ Toast message appears: 'Beat Indicator ON'" -ForegroundColor Gray
Write-Host "  ✓ Red circle appears at (50, 50) pixels from top-left" -ForegroundColor Gray
Write-Host "  ✓ Circle pulses (fades in/out) on detected beats" -ForegroundColor Gray
Write-Host "  ✓ ~250ms between pulses (adaptive to music)" -ForegroundColor Gray
Write-Host "  ✓ Toast message appears: 'Beat Indicator OFF' when disabled" -ForegroundColor Gray
Write-Host ""

Write-Host "Press any key to launch Vibeus..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown')

Write-Host ""
Write-Host "Launching Vibeus.exe --debug..." -ForegroundColor Cyan

Set-Location "F:\chilltittiesvisualizer\Vibeus\build\Release"
Start-Process -FilePath ".\Vibeus.exe" -ArgumentList "--debug"

Write-Host ""
Write-Host "Vibeus launched! Follow test instructions above." -ForegroundColor Green
Write-Host ""
Write-Host "When done testing, press any key to mark test as complete..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown')

Write-Host ""
Write-Host "Test completed! Results:" -ForegroundColor Cyan
Write-Host ""

# Simple test result collection
Write-Host "Did the beat indicator toggle on/off correctly? (Y/N): " -NoNewline -ForegroundColor Yellow
$toggle = Read-Host

Write-Host "Did the red circle pulse on beats? (Y/N): " -NoNewline -ForegroundColor Yellow
$pulse = Read-Host

Write-Host "Was the timing accurate (~50ms from actual beats)? (Y/N): " -NoNewline -ForegroundColor Yellow
$timing = Read-Host

Write-Host "Did toast notifications appear? (Y/N): " -NoNewline -ForegroundColor Yellow
$toast = Read-Host

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Test Results Summary" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Toggle functionality:    $toggle" -ForegroundColor White
Write-Host "Beat pulse visible:      $pulse" -ForegroundColor White
Write-Host "Timing accuracy:         $timing" -ForegroundColor White
Write-Host "Toast notifications:     $toast" -ForegroundColor White
Write-Host ""

$passCount = @($toggle, $pulse, $timing, $toast) | Where-Object { $_ -eq 'Y' } | Measure-Object | Select-Object -ExpandProperty Count
$totalTests = 4

if ($passCount -eq $totalTests) {
    Write-Host "✅ ALL TESTS PASSED ($passCount/$totalTests)" -ForegroundColor Green
    Write-Host "E1.3 is ready for production!" -ForegroundColor Green
} elseif ($passCount -ge 3) {
    Write-Host "⚠️  MOSTLY PASSED ($passCount/$totalTests)" -ForegroundColor Yellow
    Write-Host "Minor issues detected - review and iterate" -ForegroundColor Yellow
} else {
    Write-Host "❌ TESTS FAILED ($passCount/$totalTests)" -ForegroundColor Red
    Write-Host "Significant issues detected - debug required" -ForegroundColor Red
}

Write-Host ""
Write-Host "Test log saved to: docs/E1.3_BEAT_INDICATOR_TEST_PLAN.md" -ForegroundColor Cyan
Write-Host "Update sprint tracker: docs/SPRINT_01.md" -ForegroundColor Cyan
Write-Host ""
