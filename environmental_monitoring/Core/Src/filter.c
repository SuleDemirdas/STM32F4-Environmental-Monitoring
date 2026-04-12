void bubble_sort(float* array, uint8_t array_size)
{
    float temp;
    for (uint8_t i = 0; i < array_size - 1; i++) {
        for (uint8_t j = 0; j < array_size - i - 1; j++) {
            if (array[j] > array[j + 1]) {
            	temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

float calculate_median(float* array, uint8_t array_size)
{
	if((array_size % 2 == 0))
	{
		return (array[array_size / 2] + array[(array_size / 2) - 1]) / 2;
	}
	else
	{
		return array[(array_size - 1) / 2];
	}
}
