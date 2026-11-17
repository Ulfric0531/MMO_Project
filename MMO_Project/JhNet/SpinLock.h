#pragma once

void AcquireSpinLock(volatile long* lock);

void ReleaseSpinLock(volatile long* lock);
